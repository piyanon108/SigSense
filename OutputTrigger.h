#ifndef OUTPUTTRIGGER_H
#define OUTPUTTRIGGER_H

#include <QThread>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QString>
#include <QCoreApplication>
#include <QtGlobal>
#include <ChatServer.h>
#include <gpiod.h>
#include <iostream>
#include <cstdlib>

#define GPIO_CHIP "gpiochip2"
#define GPIO_LINE 0
#ifndef	CONSUMER
#define	CONSUMER	"Consumer"
#endif

class OutputTrigger : public QObject
{
    Q_OBJECT
public:
    explicit OutputTrigger(QObject *parent = nullptr);
    void keymasterPress(QString message);
signals:

private:
    int setRelayTrigger();

private slots:



};

#endif // OUTPUTTRIGGER_H
