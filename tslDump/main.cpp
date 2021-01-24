#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#ifdef QT_SERIALPORT_LIB
#include <QSerialPortInfo>
#include <QSerialPort>
#endif
#include <QUdpSocket>
#include <tsl.h>
#include <iostream>
#include <tsllogger.h>
int main(int argc, char *argv[])
{
	QString firstSerialPort;
#ifdef QT_SERIALPORT_LIB
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	if (ports.size())
		firstSerialPort = ports.first().portName();
#endif
	QCoreApplication a(argc, argv);
	QCommandLineOption optColorTally(
	            QStringList()
	            << "c"
	            << "color",
	            QStringLiteral("Colorize tally"));
	QCommandLineOption optVerbose(
	            QStringList()
	            << "V"
	            << "verbose",
	            QStringLiteral("verbose output"));
	QCommandLineOption optListen(
	            QStringList()
	            << "L"
	            << "listen",
	            QStringLiteral("listen to tcp/upd port"),
	            QStringLiteral("[addr:]port"),
	            QStringLiteral("1234")
	            );
	QCommandLineOption optBaud(
	            QStringList()
	            << "b"
	            << "baud",
	            QStringLiteral("serial port baud rate (default: 38400)"),
	            QStringLiteral("baud"),
	            QStringLiteral("38400")
	            );
	QCommandLineOption optTcpClient(
	            QStringList()
	            << "connect-tcp",
	            QStringLiteral("connect to tcp port (client mode)"),
	            QStringLiteral("[addr:]port"),
	            QStringLiteral("1234")
	            );
	QCommandLineOption optSerialPort(
	            QStringList()
	            << "p"
	            << "serial",
	            QStringLiteral("listen to serial port"),
	            QStringLiteral("port name"),
	            firstSerialPort
	            );
	QCommandLineOption optUDP(
	            QStringList()
	            << "U"
	            << "udp",
	            QStringLiteral("listen to UDP port")
	            );
	QCommandLineOption optDebug(
	            QStringList()
	            << "D"
	            << "debug",
	            QStringLiteral("debug message processing")
	            );
	QCommandLineOption optTimestamp(
	            QStringList()
	            << "T"
	            << "timestamp",
	            QStringLiteral("print timestamps")
	            );
	QCommandLineOption optCSV(
	            QStringList()
	            << "csv",
	            QStringLiteral("csv output")
	            );
	QCommandLineOption optFilterRepeats(
	            QStringList()
	            << "1"
	            << "filter-retransmit",
	            QStringLiteral("Filter out any re-transmissions (show every transition only once)")
	            );
	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption(optColorTally);
	parser.addOption(optVerbose);
	parser.addOption(optListen);
	parser.addOption(optTcpClient);
	parser.addOption(optUDP);
#ifdef QT_SERIALPORT_LIB
	parser.addOption(optSerialPort);
	parser.addOption(optBaud);
#endif
	parser.addOption(optDebug);
	parser.addOption(optTimestamp);
	parser.addOption(optCSV);
	parser.process(a);
	enum {
		modeInvalid = -1,
		modeUDP=1,
		modeTcpServer=2,
		modeTcpClient=3,
		modeSerial=4
	};
	int mode=0;
	QSerialPort serial;
	QHostAddress listenTo = QHostAddress::Any;
	int networkPort = 1234;
	if (parser.isSet(optListen))
	{
		bool useUDP = false;
		if (parser.isSet(optUDP))
			useUDP = true;
		listenTo = QHostAddress::Any;
		QStringList parts = parser.value(optListen).split(':');
		if (parts.empty())
			parser.showHelp(1);
		networkPort = parts.takeLast().toInt();
		if (!networkPort)
			parser.showHelp(1);
		if (parts.size())
			listenTo = QHostAddress(parts.takeLast());
		if (listenTo.isNull())
			parser.showHelp(1);
		if (parts.size())
		{
			QString protocol = parts.takeLast();
			if (protocol.toLower() == "udp")
				useUDP = true;
		}
		if (useUDP)
			mode = modeUDP;
		else
		{
			mode = parser.isSet(optTcpClient)?modeTcpClient:modeTcpServer;
		}
	}
	else
	{
		if (parser.isSet(optSerialPort))
		{
			mode = modeSerial;
			serial.setPortName(parser.value(optSerialPort));
			serial.setBaudRate(parser.value(optBaud).toInt());
			serial.setParity(QSerialPort::EvenParity);
		}
	}

	TSL tsl;
	QUdpSocket udpSocket;

	switch(mode)
	{
		case modeSerial:
			if (!serial.open(QIODevice::ReadWrite))
			{
				std::cerr << "Could not open serial port: "
				          << serial.portName().toStdString()
				          << " with baudrate "
				          << serial.baudRate()
				          << ":\n   "
				          << serial.errorString().toStdString()
				          << std::endl;
				parser.showHelp(1);
			}
			else
			{
				std::cerr << "TSL-Monitor listening on "
				          << serial.portName().toStdString()
				          << "\n";
			}
			tsl.setChannel(&serial);
			break;
		case modeUDP:
			if (!udpSocket.bind(listenTo, networkPort))
			{
				std::cerr << "Could not bind to udp port: "
				          << listenTo.toString().toStdString()
				          << ":" << networkPort
				          << "\nError: "
				          << udpSocket.errorString().toStdString()
				          << std::endl;
				parser.showHelp(1);
			}
			else
			{
				std::cerr << "TSL-Monitor listening on UDP port "
				          << networkPort
				          << "\n";
			}
			tsl.setUdpSocket(&udpSocket);
			break;
		case modeInvalid:
		default:
			parser.showHelp(1);
			break;

	}
	tslLogger logger;
	QObject::connect(
	            &tsl, SIGNAL(tslReceived(int,QString,int,int)),
	            &logger, SLOT(onTSLMessage(int,QString,int,int)));

	if (parser.isSet(optDebug))
		tsl.setShowDebug(true);

//	tsl.setShowDebug(true);
//	tsl.processByte(0x82);
//	tsl.processByte(0x7f);
//	for (int i=0;i<16;++i)
//		tsl.processByte('a' + i);

	return a.exec();
}
