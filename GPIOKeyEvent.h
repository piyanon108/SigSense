#ifndef GPIOKEYEVENT_H
#define GPIOKEYEVENT_H

#include <QThread>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QString>
#include <QCoreApplication>
#include <QtGlobal>
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <iostream>
#include <sys/time.h>

#include <cstdint>  // สำหรับ C++11 หรือสูงกว่า

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"
#endif

#define INPUTGPIOCHIP "gpiochip3"
#define INPUTGPIOLINE 1

class GPIOKeyEvent : public QThread
{
    Q_OBJECT
public:
    explicit GPIOKeyEvent(QObject *parent = nullptr) : QThread(parent){}

signals:
    void keypress(quint32 nanosec);

protected:
    void run();
};

#endif // GPIOKEYEVENT_H
