#include "tsllogger.h"
#include <iomanip>
#include <iostream>
#include <QDateTime>
tslLogger::tslLogger(QObject *parent) : QObject(parent),
    mCSV(false),
    mShowTS(true)
{

}

void tslLogger::onTSLMessage(int addr, QString text, int tally, int brightness)
{
	if (mCSV)
	{
		QString out;
		out += QDateTime::currentDateTimeUtc()
		       .toString("yyyy-MM-dd HH:mm:ss+zzz");
		out +=";";
		out += QString::asprintf("%03d;%01d;%s%s%s%s;",
		                         addr, brightness,
		                         tally&1?"1":"0",
		                         tally&2?"1":"0",
		                         tally&4?"1":"0",
		                         tally&8?"1":"0");
		out+="\"" + text + "\"";
		std::cout << out.toStdString() << std::endl;
	}
	else
	{
		if (mShowTS)
			std::cout << "["
			          << QDateTime::currentDateTimeUtc().toString(
			                 "yyyy-MM-dd HH:mm:ss+zzz").toStdString()
			          << "] ";
		std::cout << "A" << std::setw(3) << std::setfill('0') << addr;
		std::cout << " B" << std::setw(1) << brightness;

		std::cout << " ["
		          << ((tally & 1)?1:0)
		          << ((tally & 2)?1:0)
		          << ((tally & 4)?1:0)
		          << ((tally & 8)?1:0)
		          << "] ";
		std::cout << text.toStdString();
		std::cout << std::endl;
	}
}
