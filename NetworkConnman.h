#ifndef NETWORKCONNMAN_H
#define NETWORKCONNMAN_H

#include <QString>
#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QFile>
class NetworkConnman : public QObject
{
    Q_OBJECT
public:
    explicit NetworkConnman(QString phyNetworkName, QObject *parent = nullptr);
    struct phyNetwork{
        QString dhcpmethod = "";
        QString ipaddress = "";
        QString subnet = "";
        QString gateway = "";
        QString dnsList = "";
        QString pridns = "";
        QString secdns = "";
        QString macAddress = "";
        QString serviceName = "";
        QString phyNetworkName = "";
        bool Available = false;
    };
    phyNetwork *network;

    QProcess* getAddressProcess;
    void connmanSetStaticIP(QString ipaddr, QString netmask, QString gateway, QString dns1, QString dns2);
    void connmanSetDHCP();
    void checkCard();
    void getAddress();
    void resetNetwork();
    void setNTPServer(QString ntpServer);
    void resetConnman();
    QString getChronycSources();
    QStringList getChronycSourcesCSV();
    QStringList getChronycClientsCSV();
    QString getUPTime();
    double getCPUTemp();
    float getCPUUsage();
    int getNetworkCarrier();
    float getMemUsage();
    void setBondAddress(QString ipaddr,QString netmask,QString gateway,QString dns1,QString dns2);
    void setNetworkName(QString dns1,QString dns2);
signals:
    void lanPlugin(QString phyNetworkName);
    void lanRemove(QString phyNetworkName);
    void onNewAddress(QString networkInfo);
public slots:

private:
    QString getWiredService();
    QString getMacAddress();
    QString readLine(QString fileName);

    QProcess* getSystemInfoProcess;
    QProcess* setNetworkProcess;
    QProcess* getChronycProcess;
    QProcess* getSysInfoProcess;
    int last_worktime, last_idletime;
};

#endif // NETWORKCONNMAN_H
