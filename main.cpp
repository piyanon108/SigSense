#include <QCoreApplication>
#include "StartRelayTrigger.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    StartRelayTrigger runapp;
    return a.exec();
}
