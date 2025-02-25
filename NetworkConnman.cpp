#include "NetworkConnman.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <QThread>
using namespace std;
NetworkConnman::NetworkConnman(QString phyNetworkName, QObject *parent) : QObject(parent)
{
    getAddressProcess = new QProcess(this);
    getSystemInfoProcess = new QProcess(this);
    setNetworkProcess = new QProcess(this);
    getChronycProcess = new QProcess(this);
    getSysInfoProcess = new QProcess(this);
    network = new phyNetwork;
    network->phyNetworkName = phyNetworkName;
    network->macAddress = getMacAddress();
    network->serviceName = getWiredService();
}

QString NetworkConnman::getWiredService()
{
    QString service = "";
    QString prog = "/bin/bash";
    QStringList arguments;
    QString macAddress = network->macAddress;
    macAddress = macAddress.replace(":","");
    arguments << "-c" << QString("connmanctl services | grep -i %1 | awk '{print $3}'").arg(macAddress);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    service = QString(getAddressProcess->readAll()).trimmed();
    arguments.clear();
    return service;
}
QString NetworkConnman::getChronycSources()
{
    if (!(getChronycProcess->state() == QProcess::NotRunning))
        return "";
    QString prog = "/bin/bash";
    QStringList arguments;
    arguments << "-c" << QString("chronyc sources | grep '*' |  awk '{print $2}'");
    getChronycProcess->start(prog , arguments);
    getChronycProcess->waitForFinished();
    QString chronyc_sources = QString(getChronycProcess->readAll()).trimmed();
//    qDebug() << "chronyc sources" << chronyc_sources;
    return chronyc_sources;
}
QStringList NetworkConnman::getChronycSourcesCSV()
{
    system("chronyc -c sources > /etc/ChronycSources.csv");
    QString fileName = QString("/etc/ChronycSources.csv");
    QStringList csv;
    std::ifstream file(fileName.toStdString());
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // using printf() in all tests for consistency
            csv << QString::fromStdString(line);
        }
        file.close();
    }
    return csv;
}
QStringList NetworkConnman::getChronycClientsCSV()
{
    system("chronyc -c clients > /etc/ChronycClients.csv");
    QString fileName = QString("/etc/ChronycClients.csv");
    QStringList csv;
    std::ifstream file(fileName.toStdString());
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // using printf() in all tests for consistency
            csv << QString::fromStdString(line);
        }
        file.close();
    }
    return csv;
}
QString NetworkConnman::getMacAddress()
{
    QString prog = "/bin/bash";
    QStringList arguments;
    arguments << "-c" << QString("ifconfig %1 | grep 'ether'  | awk '{print $2}'").arg(network->phyNetworkName);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    network->macAddress = QString(getAddressProcess->readAll()).trimmed();
    qDebug() << "network->macAddress" << network->macAddress;
    return network->macAddress;
}
double NetworkConnman::getCPUTemp()
{
    QString fileName = "/sys/bus/iio/devices/iio:device0/in_temp0_ps_temp_raw";
    double val1;
    string getval_str = fileName.toStdString();
    ifstream getval(getval_str.c_str());
    if (getval.fail()){
        cout << " OPERATION FAILED: Unable to getCPUUsage." << endl;
        return 0;
            }

    getval >> val1;
    getval.close(); //close the value file
    return val1/1000.0;
}
QString NetworkConnman::getUPTime()
{
    system("uptime -p > /etc/uptime");
    QString fileName = QString("/etc/uptime");
    return readLine(fileName);
}
void NetworkConnman::getAddress()
{
    QString networkInfo;
    network->serviceName = getWiredService();
    if (network->serviceName == "")
    {
        network->ipaddress = "";
        network->subnet = "";
        network->gateway = "";
        network->dnsList = "";
        networkInfo = QString("{"
                              "\"menuID\"       :\"addressInfo\", "
                              "\"IPAddress\"    :\"%1\", "
                              "\"Netmask\"      :\"%2\", "
                              "\"Gateway\"      :\"%3\", "
                              "\"DNSServer\"    :\"%4\", "
                              "\"MACAddress\"   :\"%5\", "
                              "\"DHCPMethod\"   :\"%6\" "
                              "}").arg(network->ipaddress).arg(network->subnet).arg(network->gateway).arg(network->dnsList).arg(network->macAddress).arg(network->dhcpmethod);

        emit onNewAddress(networkInfo);
        return;
    }
    QString prog = "/bin/bash";
    QStringList arguments;
    qDebug() << "getAddress";

    arguments << "-c" << QString("ifconfig %1 | grep 'inet ' | awk '{print $2}' | sed 's/addr://'").arg(network->phyNetworkName);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    network->ipaddress = QString(getAddressProcess->readAll()).trimmed();
    qDebug() << "netWorkCardAddr" << network->ipaddress;

    arguments.clear();
    arguments << "-c" << QString("ifconfig %1 | grep 'inet ' | awk '{print $4}' | sed 's/Mask://'").arg(network->phyNetworkName);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    network->subnet = QString(getAddressProcess->readAll()).trimmed();
    qDebug() << "netWorkCardMask" << network->subnet;

    arguments.clear();
    arguments << "-c" << QString("connmanctl services %1 | grep \"IPv4 =\" | grep -oP '(?<=\\[).*(?=\\])'").arg(network->serviceName);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    QString output = QString(getAddressProcess->readAll()).trimmed();
    QStringList data = output.split(",");
    qDebug() << "output" << output << data;

    Q_FOREACH(QString networkData, data){
        if (networkData.contains("Gateway")){
            qDebug() << networkData;
            network->gateway = networkData.split("=").at(1);
            qDebug() << "netWorkCardGW" << network->gateway;
        }
    }

    arguments.clear();
    arguments << "-c" << QString("connmanctl services %1 | grep \"Nameservers =\" | grep -oP '(?<=\\[).*(?=\\])'").arg(network->serviceName);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    network->dnsList = QString(getAddressProcess->readAll()).trimmed();
    network->dnsList = network->dnsList.replace(" ","");
    network->dnsList = network->dnsList.replace(","," ");


    qDebug() << "netWorkCardDNS" << network->dnsList;

    networkInfo = QString("{"
                          "\"menuID\"       :\"addressInfo\", "
                          "\"IPAddress\"    :\"%1\", "
                          "\"Netmask\"      :\"%2\", "
                          "\"Gateway\"      :\"%3\", "
                          "\"DNSServer\"    :\"%4\", "
                          "\"MACAddress\"   :\"%5\", "
                          "\"DHCPMethod\"   :\"%6\" "
                          "}").arg(network->ipaddress).arg(network->subnet).arg(network->gateway).arg(network->dnsList).arg(network->macAddress).arg(network->dhcpmethod);

    arguments.clear();
    emit onNewAddress(networkInfo);
    qDebug() << networkInfo;
}
void NetworkConnman::checkCard()
{
    QString command;
    QString macAddress;

    macAddress = network->macAddress;
    network->serviceName = getWiredService();



    QString prog = "/bin/bash";//shell
    QStringList arguments;

    if ((network->Available) & (network->serviceName == ""))
    {
        network->Available = false;
        emit lanRemove(network->phyNetworkName);
    }
    else if ((network->Available == false) & (network->serviceName != ""))
    {
        network->Available = true;
        qDebug() << "lanPlugin" << network->phyNetworkName << network->serviceName << network->Available;
        emit lanPlugin(network->phyNetworkName);
    }
}
void NetworkConnman::connmanSetDHCP()
{
    QString command;
    network->serviceName = getWiredService();

    QString prog = "/bin/bash";//shell
    QStringList arguments;

    if (network->serviceName == "") return;
    command = QString("connmanctl config %1 ipv4 dhcp").arg(network->serviceName);
    arguments << "-c" << command;
    setNetworkProcess->start(prog , arguments);
    setNetworkProcess->waitForFinished();
    arguments.clear();

    command = QString("ifdown %1").arg(network->phyNetworkName);
    arguments << "-c" << command;
    setNetworkProcess->start(prog , arguments);
    setNetworkProcess->waitForFinished();
    arguments.clear();

    command = QString("ifup %1").arg(network->phyNetworkName);
    arguments << "-c" << command;
    setNetworkProcess->start(prog , arguments);
    setNetworkProcess->waitForFinished();
    arguments.clear();

}

void NetworkConnman::setNTPServer(QString ntpServer)
{
    QString command;
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    if (network->serviceName == "") return;
    command = QString("connmanctl config %1 --timeservers %2").arg(network->serviceName).arg(ntpServer);
    arguments << "-c" << command;
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    arguments.clear();
}

void NetworkConnman::resetConnman()
{
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    qDebug() << "networking restarting";
    arguments << "-c" << QString("/etc/init.d/connman restart");
    setNetworkProcess->start(prog , arguments);
    setNetworkProcess->waitForFinished();
    arguments.clear();
    qDebug() << "networking restarted";

}
void NetworkConnman::connmanSetStaticIP(QString ipaddr,QString netmask,QString gateway,QString dns1,QString dns2)
{
    QString command;
    network->serviceName = getWiredService();

    QString prog = "/bin/bash";//shell
    QStringList arguments;

    if (network->serviceName == "") return;
    command = QString("connmanctl config %1 ipv4 dhcp").arg(network->serviceName);

    if (network->serviceName == "") return;
    if (netmask == "0.0.0.0") netmask = "255.255.255.0";
    if (gateway == "0.0.0.0") gateway = "";
    if (dns1 == "0.0.0.0") dns1 = "";
    if (dns2 == "0.0.0.0") dns2 = "";
    if(ipaddr!="0.0.0.0")
    {
        command = QString("connmanctl config %1 --ipv4 manual %2 %3 %4").arg(network->serviceName).arg(ipaddr).arg(netmask).arg(gateway);
        qDebug() << command;
        arguments << "-c" << command;
        setNetworkProcess->start(prog , arguments);
        setNetworkProcess->waitForFinished();
        arguments.clear();

        command = QString("connmanctl config %1 --nameservers %2 %3").arg(network->serviceName).arg(dns1).arg(dns2);
        qDebug() << command;
        arguments << "-c" << command;
        setNetworkProcess->start(prog , arguments);
        setNetworkProcess->waitForFinished();
        arguments.clear();

        QString ifconfigCommand = QString("ifconfig %1 %2").arg(network->phyNetworkName).arg(ipaddr);
        system(ifconfigCommand.toStdString().c_str());
    }
    else
    {
        command = QString("connmanctl config %1 ipv4 dhcp").arg(network->serviceName);
        qDebug() << command;
        arguments << "-c" << command;
        setNetworkProcess->start(prog , arguments);
        setNetworkProcess->waitForFinished();
        arguments.clear();
    }
}
void NetworkConnman::resetNetwork()
{
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    qDebug() << "networking restarting";
    QString command = QString("ifdown %1").arg(network->phyNetworkName);
    arguments << "-c" << command;
    setNetworkProcess->start(prog , arguments);
    setNetworkProcess->waitForFinished();
    arguments.clear();

    command = QString("ifup %1").arg(network->phyNetworkName);
    arguments << "-c" << command;
    setNetworkProcess->start(prog , arguments);
    setNetworkProcess->waitForFinished();
    arguments.clear();

    arguments.clear();
    qDebug() << "networking restarted";

}
QString NetworkConnman::readLine(QString fileName)
{
    QFile inputFile(fileName);
    inputFile.open(QIODevice::ReadOnly);
    if (!inputFile.isOpen())
        return "";

    QTextStream stream(&inputFile);
    QString line = stream.readLine();
    inputFile.close();
//    qDebug() << line;
    return line.trimmed();
}
float NetworkConnman::getMemUsage(){
    QString memInfo;
    int totalMem;
    int useMem;
    QString arguments = QString("free | grep Mem | awk '{print $2,$3}' > /opt/sysinfo/memInfo");
    system(arguments.toStdString().c_str());
    memInfo = readLine("/opt/sysinfo/memInfo");
    arguments.clear();
    if(memInfo.split(" ").size() >= 2)
    {
        totalMem = QString(memInfo.split(" ").at(0)).toInt();
        useMem = QString(memInfo.split(" ").at(1)).toInt();
    }
    if(totalMem > useMem){
        return float((useMem*100.0)/totalMem);
    }
    return 0.0;
}

float NetworkConnman::getCPUUsage(){
    float rate;
    string val0;
    long val1, val2, val3, val4, val5;
    string getval_str = "/proc/stat";
    ifstream getval(getval_str.c_str());
    if (getval.fail()){
        cout << " OPERATION FAILED: Unable to getCPUUsage." << endl;
        return 0.0;
            }

    getval >> val0 >> val1 >> val2 >> val3 >> val4 >> val5 ;
    getval.close(); //close the value file
    if (QString::fromStdString(val0) == "cpu"){
        long worktime, idletime;
        worktime = val1+val2+val3;
        idletime = val4;
        float dworktime = (worktime-last_worktime);
        float didletime = (idletime-last_idletime);
        rate = float(dworktime)/(didletime+dworktime);
        last_worktime=worktime;
        last_idletime=idletime;
        if(last_worktime==0)
            return 0;
    }
    else
    {
        return 0.0;
    }
    if (rate < 0) rate = rate*(-1);
    return rate*100.0;
}
int NetworkConnman::getNetworkCarrier()
{
    QString fileName = QString("/sys/class/net/%1/carrier").arg(network->phyNetworkName);
    int val1;
    string getval_str = fileName.toStdString();
    ifstream getval(getval_str.c_str());
    if (getval.fail()){
        cout << " OPERATION FAILED: Unable to getCPUUsage." << endl;
        return 0;
            }

    getval >> val1;
    getval.close(); //close the value file
    return val1;
}
void NetworkConnman::setNetworkName(QString dns1,QString dns2)
{
    QString filename = "/etc/resolv.conf";
    QString data;
    if (dns1 == "0.0.0.0") dns1 = "";
    if (dns2 == "0.0.0.0") dns2 = "";
    if ((dns1 != "") & (dns2 != ""))
        data = QString("nameserver %1\n"
                       "nameserver %2\n").arg(dns1).arg(dns2);
    else if ((dns1 != "") & (dns2 == ""))
        data = QString("nameserver %1\n")
                .arg(dns1);
    else if ((dns1 == "") & (dns2 != ""))
        data = QString("nameserver %1\n")
                .arg(dns2);
    else data = "";

    QByteArray dataAyyay(data.toLocal8Bit());
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << dataAyyay;
    file.close();
}

void NetworkConnman::setBondAddress(QString ipaddr,QString netmask,QString gateway,QString dns1,QString dns2)
{
    system("systemctl stop connman");
    system("/etc/init.d/networking stop");
    QString filename = "/etc/network/interfaces";
    QString data;
    QString nameserver = "";
    if ((netmask == "") || (netmask == "0.0.0.0")) netmask = "netmask 255.255.255.0";
    else  netmask = QString("netmask %1").arg(netmask);

    if ((gateway == "0.0.0.0") || (gateway == "")) gateway = "#gateway" ;
    else  gateway = QString("gateway %1").arg(gateway);

    if ((dns1 != "") & (dns2 != ""))
        nameserver = QString("dns-nameservers %1 %2").arg(dns1).arg(dns2);
    else if (!((dns1 == "") || (dns1 == "0.0.0.0")))
        nameserver = QString("dns-nameservers %1").arg(dns1);
    else if (!((dns2 != "") || (dns2 != "0.0.0.0")))
        nameserver = QString("dns-nameservers %1").arg(dns2);
    else
        nameserver = "";

    if ((ipaddr != "") && (ipaddr != "0.0.0.0")) ipaddr = QString("address %1").arg(ipaddr);

    if ((ipaddr != "") && (ipaddr != "0.0.0.0")  && (netmask != "")  && (gateway != ""))
    {
        data = QString( "# /etc/network/interfaces -- configuration file for ifup(8), ifdown(8)\n"
                    "# The loopback interface\n"
                    "auto lo\n"
                    "iface lo inet loopback\n"
                    "auto bond0\n"
                    "iface bond0 inet static\n"
                    "  %1\n"
                    "  %2\n"
                    "  %3\n"
                    "  #%4\n"
                    "  bond-mode 4\n"
                    "  bond-miimon 100\n"
                    "  bond-slaves eth0 eth1\n"
                    "  bond-ad_select bandwidth"
                    "\n"
                    "auto eth0\n"
                    "  iface eth0 inet dhcp\n"
                    "  bond-master bond0\n"
                    "  bond-primary eth0\n"
                    "\n"
                    "auto eth1\n"
                    "  iface eth1 inet dhcp\n"
                    "  bond-master bond0\n").arg(ipaddr).arg(netmask).arg(gateway).arg(nameserver);
    }
    else
    {
        data = QString( "# /etc/network/interfaces -- configuration file for ifup(8), ifdown(8)\n"
                    "# The loopback interface\n"
                    "auto lo\n"
                    "iface lo inet loopback\n"
                    "auto bond0\n"
                    "iface bond0 inet dhcp\n"
                    "  bond-mode 4\n"
                    "  bond-miimon 100\n"
                    "  bond-slaves eth0 eth1\n"
                    "\n"
                    "auto eth0\n"
                    "  iface eth0 inet dhcp\n"
                    "  bond-master bond0\n"
                    "\n"
                    "auto eth1\n"
                    "  iface eth1 inet dhcp\n"
                    "  bond-master bond0\n");
    }

    QByteArray dataAyyay(data.toLocal8Bit());
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << dataAyyay;
    file.close();
    system("/etc/init.d/networking start");
    QThread::sleep(1);
    system("/etc/init.d/networking restart");
    setNetworkName( dns1, dns2);
}
