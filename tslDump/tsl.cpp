#include "tsl.h"
#include <QDebug>
#include <QUdpSocket>
#include <QNetworkDatagram>

enum parserState
{
	tslInit=0,
	tslControl=1,
	tslData=2
};


static QString hexdump(const QByteArray & ba)
{
	QString ret;
	for(int i=0;i<ba.size(); ++i)
	{
		if (i)
			ret+=" ";
		ret += QString::asprintf("%02x", ba.at(i) & 0xff);
	}
	return ret;
}

TSL::TSL(QObject *parent) : QObject(parent)
{
	mParserState = tslInit;
	mShowDebug = false;
	mChannel = nullptr;
	mDgramSocket = nullptr;
}

void TSL::setChannel(QIODevice *chan)
{
	mUdpDestPort = -1;
	mDgramSocket = nullptr;
	mChannel = chan;
	disconnect(this, SLOT(onReadyRead()));
	connect(mChannel, SIGNAL(readyRead()), SLOT(onReadyRead()));
}

void TSL::setUdpSocket(QUdpSocket *sock)
{
	mDgramSocket = sock;
	mChannel = nullptr;
	disconnect(this, SLOT(onReadyRead()));
	connect(mDgramSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
}

void TSL::setUdpTarget(const QHostAddress &addr, int port)
{
	mChannel = nullptr;
	mHostAddr = addr;
	mUdpDestPort = port;
}

bool TSL::showDebug() const
{
	return mShowDebug;
}

void TSL::setShowDebug(bool showDebug)
{
	mShowDebug = showDebug;
}

void TSL::processByte(unsigned char byte, bool flush)
{
	//	if (mShowDebug)
	//		qDebug().noquote()
	//		        << "rx:"
	//		        << QString::asprintf("0x%02x", byte  & 0xff);

	if (flush)
	{
		if (mParserState != tslInit)
		{
			if (showDebug())
			{
				qDebug().noquote()
				        << "(!!) Resetting parser within packet:"
				        << hexdump(mPacketBuffer);
			}
			else
			{
				qDebug().nospace()
				        << "(!!) "
				        << "Resetting parser within packet ("
				        << mPacketBuffer.size()
				        << " bytes in buffer)";
			}
		}
		mParserState = tslInit;
		return;
	}

	if (byte & 0x80)
	{
		if ((mParserState != tslInit) && mShowDebug)
		{
			qDebug()<<"starting new tsl packet with parser not expecting it";
		}
		mParserState = tslControl;
		mPacketBuffer.clear();
		mPacketBuffer.push_back(byte);
	}
	else
	{
		switch(mParserState)
		{
			case tslControl:
				//store control byte
				mPacketBuffer.push_back(byte);

				mByteCounter = 0;
				mParserState = tslData;
				break;
			case tslData:
				mPacketBuffer.push_back(byte);

				if (++mByteCounter == 16)
				{
					processFrame(mPacketBuffer);
					mParserState = tslInit;
				}
				break;
			default:
				if (mShowDebug)
				{
					qDebug().noquote()
					        << "received non-start byte while not expecting it:"
					        << QString::asprintf("0x%02x", byte);
				}
				mParserState = tslInit;
				break;
		}
	}

}

void TSL::forceFlushBuffer()
{
	processByte(0, true);
}

QByteArray TSL::assembleMessage(int addr, QString text, int tally, int brightness)
{
	QByteArray packet;
	//addr byte (0x80 set for start of frame)
	packet.push_back(0x80 | (addr & 0x7f));
	//control byte
	packet.push_back((tally & 0x0f) | ((brightness & 0x03) << 4));
	QByteArray textData = text.toLatin1();
	while(textData.size() < 16)
		textData.append(' ');
	//limit to safe character set
	for(int i=0;i<16;++i)
	{
		unsigned char c = textData[i]&0x7f;
		if ((c>=31) && (c<128))
			packet.push_back(c);
		else
			packet.push_back(' ');
	}
	return packet;
}

void TSL::sendTSL(int addr, QString text, int tally, int brightness)
{
	QByteArray packet = assembleMessage(addr, text, tally, brightness);
	if (packet.size()!=18)
	{
		qCritical()<<"Packet assembly failed (length mismatch)";
		return;
	}
	if (mShowDebug)
	{
		qDebug().noquote().nospace()
		        << "Send:"
		        << " 0x" << QString::asprintf("%02x", packet.at(0) & 0xff)
		        << " 0x" << QString::asprintf("%02x", packet.at(1) & 0xff)
		        << " \"" << QString::fromLatin1(packet.mid(2,16)) << "\"";
	}
	if (mChannel)
	{
		mChannel->write(packet);
	}
	if (mDgramSocket)
	{
		mDgramSocket->writeDatagram(packet, mHostAddr, mUdpDestPort);
	}
}

void TSL::onReadyRead()
{
	if (mDgramSocket)
	{
		while(mDgramSocket->hasPendingDatagrams())
		{
			QNetworkDatagram dgram = mDgramSocket->receiveDatagram();
			if (!dgram.isValid())
				continue;
			QByteArray data = dgram.data();
			for (int i=0;i<data.size();++i)
				processByte(data.at(i));
			forceFlushBuffer();
		}
	}
	if (mChannel)
	{
		while(!mChannel->atEnd())
		{
			QByteArray data = mChannel->readAll();
			for (int i=0;i<data.size();++i)
				processByte(data.at(i));
		}
	}
}

void TSL::processFrame(const QByteArray & data)
{
	if (data.size()<18)
	{
		if (mShowDebug)
		{
			qDebug().noquote()
			        << "packet too short ("<<data.size()<<")"
			        << hexdump(data);
		}
		return;
	}
	else
	{
		if (mShowDebug)
		{
			qDebug().noquote()
			        << "packet received:"
			        << hexdump(data);
		}
		unsigned char addr = data.at(0) & 0x7f;
		unsigned char ctrl = data.at(1);
		QString text = QString::fromLatin1(data.mid(2));
		emit tslReceived(addr,
		                 text,
		                 ctrl & 0x0f,
		                 (ctrl&0x30)>>4);
//		qDebug()<<"TSL received -- "
//		       <<"Addr:"<<addr
//		      << "Tally" << (ctrl & 0x0f)
//		      << "Text:" << text;
	}
}
