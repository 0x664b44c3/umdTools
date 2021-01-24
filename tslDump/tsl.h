#ifndef TSL_H
#define TSL_H

#include <QObject>
#include <QIODevice>
#include <QHostAddress>
#include <QByteArray>
//#include <QUdpSocket>
class QUdpSocket;
class TSL : public QObject
{
	Q_OBJECT
public:
	explicit TSL(QObject *parent = nullptr);
	void setChannel(QIODevice * chan);
	void setUdpSocket(QUdpSocket * sock);
	void setUdpTarget(const QHostAddress & addr, int port);


	bool showDebug() const;
	void setShowDebug(bool showDebug);

	void processByte(unsigned char byte, bool flush = false);
	void forceFlushBuffer();
	static QByteArray assembleMessage(int addr,
	                                  QString text,
	                                  int tally=0,
	                                  int brightness=3);
signals:
	void tslReceived(int addr,
	                 QString text,
	                 int tally,
	                 int brightness);

public slots:
	void sendTSL(int addr,
	             QString text,
	             int tally,
	             int brightness);
private slots:
	void onReadyRead();
private:
	void processFrame(const QByteArray &);
	QUdpSocket *mDgramSocket;
	QIODevice * mChannel;
	QByteArray mPacketBuffer;
	QHostAddress mHostAddr;
	int mUdpDestPort;
	int mParserState;
	int mByteCounter;
	bool mShowDebug;
};

#endif // TSL_H
