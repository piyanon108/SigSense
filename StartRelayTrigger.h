#ifndef STARTRELAYTRIGGER_H
#define STARTRELAYTRIGGER_H

#include <QObject>
#include <GPIOKeyEvent.h>
#include <QDateTime>
#include <QString>
#include <SocketClient.h>
#include <OutputTrigger.h>
#include <QTimer>
#include <QSettings>
#include <QFileInfoList>
#include <QDir>
#include <QFileSystemWatcher>
#include <cstdint>
#include <NetworkConnman.h>
#include <Database.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <string>

#define FILESETTING      "/home/root/.config/PLCAnalog/setting.ini"
#define REG_DELAY_START         0x41250000 //default 2000 usec. = 2 msec. for master mode
#define REG_PULSE_PERIOD_OFFSET 0x41250008 //default 0 usec. PERIOD = 2 msec.

#define REG_ADC_READ_OFFSET     0x41270000 //default 1000 usec. = 1 msec.
#define REG_ADC_PERIOD_OFFSET   0x41270008 //default 6000 usec. = 6 msec.

#define REG_SURGE_TIMESTAMP0    0x6000000C
#define REG_SURGE_TIMESTAMP1    0x60000010
#define REG_SURGE_TIMESTAMP2    0x60000014

#define REG_SURGE_TRIGGER0      0x60000000
#define REG_SURGE_TRIGGER1      0x60000004
#define REG_SURGE_TRIGGER2      0x60000008

#define REG_MANUAL_TIMESTAMP0    0x41210008
#define REG_MANUAL_TIMESTAMP1    0x41240008
#define REG_MANUAL_TIMESTAMP2    0x41240000


#define SWVERSION "06022025 1.0"
#define HWVERSION "PLC Analog V1 2025"
#define ADCPATH "/usr/share/apache2/default-site/htdocs/adc/tmp/"

class StartRelayTrigger : public QObject
{
    Q_OBJECT
public:
    explicit StartRelayTrigger(QObject *parent = nullptr);

signals:

private:
    QJsonObject jsonObjSurgeA_Local;
    QJsonObject jsonObjSurgeA_Remote;
    QJsonObject jsonObjSurgeB_Local;
    QJsonObject jsonObjSurgeB_Remote;
    QJsonObject jsonObjSurgeC_Local;
    QJsonObject jsonObjSurgeC_Remote;
    struct phyNetwork{
        QString dhcpmethod;
        QString ipaddress;
        QString subnet;
        QString gateway;
        QString pridns;
        QString secdns;
        QString macAddress;
        QString serviceName = "";
        QString phyNetworkName = "eth0";
        bool lan_ok = false;
    };
    Database *myDatabase;
    void readFileSetting();
    GPIOKeyEvent *gpioKeyEvent = nullptr;
    OutputTrigger *outputTrigger = nullptr;
    SocketClient *slaveSocket = nullptr;    
    SocketClient *plcServerSocket = nullptr;
    ChatServer *socketServer = nullptr;
    ChatServer *websocketServer = nullptr;
    QTimer *reconnect = nullptr;
    QTimer *resetTestmode = nullptr;
    NetworkConnman *ConnmanETH0 = nullptr;
    QFileSystemWatcher *adc_file_watcher = nullptr;

    void writeRegister(uint32_t register_address, uint32_t value) ;
    quint32 readRegister(uint32_t register_address);
    void fileManager(QString sourceFile,  QString targetDir, QString date, bool clear);
    quint32 get10Nanosec(int adcNum);
    void newADCDataRecord(const QString &path, const QString &name, const QString &dir);
    void updateNetworkInfo(phyNetwork ethernet, QString phyNetworkName);
    void updateNetwork(uint8_t DHCP, QString LocalAddress, QString Netmask, QString Gateway, QString DNS1, QString DNS2,QString phyNetworkName);

    void updateSetting(QString keyMain, QString key, QVariant value);
    void senADCCommand(QString key, const QVariant &value);
    void getSystemPage(QWebSocket *webSender);
    void clearOld(const QString &path);
    void surgeEventDetect(QJsonObject jsonObjSurge, bool local);
    double calculatePosition(double deltaT, double vfac=1.0) ;


    void setcurrentDatetime(QString dateTime);
    void updateNTPServer(QString value);
    void getTimezone();
    void setLocation(QString location);

    void scanFileUpdate();
    QStringList findFile();

    bool foundfileupdate = false;
    int updateStatus = 0;
    void updateFirmware();

    QString plcServerAddress = "";
    quint64 plcServerPort = 0;
    QString adcRemoteAddress = "";
    quint64 adcRemotePort = 0;
    quint32 adc_write_offset_usec = 0;
    quint32 adc_write_period_usec = 0;

    quint32 master_start_relay_offset_usec = 0;
    quint32 slave_start_relay_offset_usec = 0;
    quint32 pulse_relay_period_offset_usec = 0;
    quint32 network_latency_usec = 0;

    quint32 surgeA_threshold = 0x2000;
    quint32 surgeB_threshold = 0x2000;
    quint32 surgeC_threshold = 0x2000;

    int adc_vref = 4096; //mV
    int adc_res = 0xFFFF;

    bool ntp = false;
    QString timeLocation = "";

    phyNetwork eth0;
    QString ntpServer = "";
    QString serialNumber = "";
    QString testMode = "Surge";
    double cableLength = 300000;
    double velocity_factor = 0.97;

    int plcInputDelay[8] = {0};

    void updatePLCInputDelay(QWebSocket *pSender);
    QString getChrrentDateTime();

private slots:
    void newMessage(QWebSocket *pSender,QJsonObject messageObj, QString message);
    void keypress(quint32 nanosec);
    void checkConnection();
    void keymasterPress(QString message);
    void keyLocalPress(QString message);
    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);
    void plcServerSocketConnected();

    void lanPlugin(QString phyNetworkName);
    void lanRemove(QString phyNetworkName);
    void resettestmode();
    void onNewClient(QWebSocket *pSender);
};

#endif // STARTRELAYTRIGGER_H
