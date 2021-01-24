#ifndef TSLLOGGER_H
#define TSLLOGGER_H

#include <QObject>

class tslLogger : public QObject
{
	Q_OBJECT
public:
	explicit tslLogger(QObject *parent = nullptr);

signals:

public slots:
	void onTSLMessage(int addr,
	             QString text,
	             int tally,
	             int brightness);
private:
	bool mCSV;
	bool mShowTS;


};

#endif // TSLLOGGER_H
