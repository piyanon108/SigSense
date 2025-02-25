#include <StartRelayTrigger.h>

void StartRelayTrigger::readFileSetting()
{
    getTimezone();
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        eth0.dhcpmethod          = settings->value("NETWORK_ETH0/DHCP","on").toString();
        eth0.ipaddress           = settings->value("NETWORK_ETH0/LocalAddress","192.168.10.21").toString();
        eth0.subnet              = settings->value("NETWORK_ETH0/Netmask","255.255.255.0").toString();
        eth0.gateway             = settings->value("NETWORK_ETH0/Gateway","").toString();
        eth0.pridns              = settings->value("NETWORK_ETH0/DNS1","").toString();
        eth0.secdns              = settings->value("NETWORK_ETH0/DNS2","").toString();

//        timeLocation            = settings->value("NETWORK/timeLocation","").toString();
        serialNumber            = settings->value("NETWORK/serialNumber","").toString();

        plcServerAddress        = settings->value("PLC/plcServerAddress","192.168.10.192").toString();
        plcServerPort           = settings->value("PLC/plcServerPort",5520).toInt();
        adcRemoteAddress        = settings->value("ADC/adcRemoteAddress","192.168.10.21").toString();
        adcRemotePort           = settings->value("ADC/adcRemotePort",8008).toInt();
        adc_write_offset_usec   = settings->value("ADC/adc_write_offset_usec",1500).toInt();
        adc_write_period_usec   = settings->value("ADC/adc_write_period_usec",6200).toInt();

        surgeA_threshold   = settings->value("ADC/surgeA_threshold",0x500).toInt();
        surgeB_threshold   = settings->value("ADC/surgeB_threshold",0x500).toInt();
        surgeC_threshold   = settings->value("ADC/surgeC_threshold",0x500).toInt();

        master_start_relay_offset_usec = settings->value("ADC/master_start_relay_offset_usec",1750).toInt();
        slave_start_relay_offset_usec  = settings->value("ADC/slave_start_relay_offset_usec",8000).toInt();
        pulse_relay_period_offset_usec = settings->value("ADC/pulse_relay_period_offset_usec",500).toInt();

        adc_vref    = settings->value("ADC/adc_vref",4096).toInt();
        adc_res     = settings->value("ADC/adc_res",0xFFFF).toInt();
        cableLength = settings->value("ADC/cableLength",300000).toInt();
        velocity_factor = settings->value("ADC/velocity_factor",0.97).toDouble();

        ntp                       = settings->value("NETWORK/ntp","false").toString() == "true";
        ntpServer                 = settings->value("NETWORK/ntpServer","pool.ntp.org").toString();
    }
    else
    {
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    qDebug() << "Loading configuration completed";
    delete settings;

    writeRegister(REG_DELAY_START,master_start_relay_offset_usec);
    writeRegister(REG_PULSE_PERIOD_OFFSET,pulse_relay_period_offset_usec);
    writeRegister(REG_ADC_READ_OFFSET,adc_write_offset_usec);
    writeRegister(REG_ADC_PERIOD_OFFSET,adc_write_period_usec);

    writeRegister(REG_SURGE_TRIGGER0,(surgeA_threshold*adc_res)/adc_vref);
    writeRegister(REG_SURGE_TRIGGER1,(surgeB_threshold*adc_res)/adc_vref);
    writeRegister(REG_SURGE_TRIGGER2,(surgeC_threshold*adc_res)/adc_vref);

}

void StartRelayTrigger::updateNetworkInfo(phyNetwork ethernet, QString phyNetworkName)
{
    QString message = QString("{\"menuID\":\"network\", \"dhcpmethod\":\"%1\", \"ipaddress\":\"%2\", \"subnet\":\"%3\", \"gateway\":\"%4\", \"pridns\":\"%5\", \"secdns\":\"%6\", \"macAddress\":\"%7\", \"phyNetworkName\":\"%8\",\"ethIndex\":0}")
            .arg(ethernet.dhcpmethod).arg(ethernet.ipaddress).arg(ethernet.subnet).arg(ethernet.gateway).arg(ethernet.pridns).arg(ethernet.secdns).arg(ethernet.macAddress).arg(phyNetworkName);
    websocketServer->broadcastMessage(message);
}

void StartRelayTrigger::lanPlugin(QString phyNetworkName)
{
    qDebug() << "lanPlugin" << phyNetworkName;
    if (phyNetworkName == "eth0"){
        qDebug() << "lan0Plugin";
        if (eth0.dhcpmethod == "on")
        {
            ConnmanETH0->getAddress();
            eth0.ipaddress = ConnmanETH0->network->ipaddress;
            eth0.subnet = ConnmanETH0->network->subnet;
            eth0.gateway = ConnmanETH0->network->gateway;
            eth0.pridns = ConnmanETH0->network->dnsList;
            eth0.macAddress = ConnmanETH0->network->macAddress;
            eth0.secdns = "";

        }
        else
        {
            ConnmanETH0->connmanSetStaticIP(eth0.ipaddress,eth0.subnet,eth0.gateway,eth0.pridns,eth0.secdns);
        }
        eth0.lan_ok = true;
        updateNetworkInfo(eth0,"eth0");
    }
}


void StartRelayTrigger::lanRemove(QString phyNetworkName)
{
    qDebug() << "lanRemove" << phyNetworkName;
    if (phyNetworkName == "eth0"){
        if (eth0.dhcpmethod == "on")
        {
            eth0.ipaddress = "";
            eth0.subnet = "";
            eth0.gateway = "";
            eth0.pridns = "";
            eth0.secdns = "";
        }
        eth0.lan_ok = false;
        updateNetworkInfo(eth0,"eth0");
    }
}

void StartRelayTrigger::updateNetwork(uint8_t DHCP, QString LocalAddress, QString Netmask, QString Gateway, QString DNS1, QString DNS2,QString phyNetworkName)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    QString strDhcpMethod = "off";
    if (DHCP) strDhcpMethod = "on";

    if(QDir::isAbsolutePath(cfgfile))
    {
        qDebug() << "Loading configuration from:" << cfgfile;
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();


        if(phyNetworkName == "eth0")
        {
            settings->setValue("NETWORK_ETH0/DHCP",strDhcpMethod);
            settings->setValue("NETWORK_ETH0/LocalAddress",LocalAddress);
            settings->setValue("NETWORK_ETH0/Netmask",Netmask);
            settings->setValue("NETWORK_ETH0/Gateway",Gateway);
            settings->setValue("NETWORK_ETH0/DNS1",DNS1);
            settings->setValue("NETWORK_ETH0/DNS2",DNS2);
        }
    }

    if (DHCP)
    {
        if(phyNetworkName == "eth0")
            ConnmanETH0->connmanSetDHCP();
    }
    else
    {
        if(phyNetworkName == "eth0")
            ConnmanETH0->connmanSetStaticIP(LocalAddress,Netmask,Gateway,DNS1,DNS2);
    }

    if(phyNetworkName == "eth0")
    {
        eth0.dhcpmethod = strDhcpMethod;
        eth0.ipaddress = LocalAddress;
        eth0.subnet = Netmask;
        eth0.gateway = Gateway;
        eth0.pridns = DNS1;
        eth0.secdns = DNS2;
        updateNetworkInfo(eth0, "eth0");
    }

    delete settings;
}

void StartRelayTrigger::updateSetting(QString keyMain, QString key, QVariant value)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    QString keySetting = QString("%1/%2").arg(keyMain).arg(key);

    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile, QSettings::IniFormat);
        settings->setValue(keySetting, value);  // Store any type of value
    }

    delete settings;
}

// void set_master_start_relay_offset_usec("master_start_relay_offset_usec",int_value_of_master_start_relay_offset_usec);
// void set_slave_start_relay_offset_usec("slave_start_relay_offset_usec",int_value_of_slave_start_relay_offset_usec);
// void set_pulse_relay_period_offset_usec("pulse_relay_period_offset_usec",int_pulse_relay_period_offset_usec);
// void set_adc_write_offset_usec("adc_write_offset_usec",int_adc_write_offset_usec);
// void set_adc_write_period_usec("adc_write_period_usec",int_adc_write_period_usec);
// void set_velocity_factor("velocity_factor",double_velocity_factor_0.xx);
// void set_cableLength("cableLength",double_cableLength_meter);
// void set_surgeA_threshold("surgeA_threshold",int_surgeA_threshold_mV);
// void set_surgeB_threshold("surgeB_threshold",int_surgeB_threshold_mV);
// void set_surgeC_threshold("surgeC_threshold",int_surgeC_threshold_mV);
// void set_adcRemoteAddress("adcRemoteAddress","string_ipaddressADCRemote:port");
// void set_plcServerAddress("plcServerAddress","string_ipaddressPlc:port");

void StartRelayTrigger::senADCCommand(QString key, const QVariant &value)
{
    QJsonObject jsonObj;
    jsonObj["objectName"] = key;
    jsonObj["value"] = QJsonValue::fromVariant(value);
    QJsonDocument jsonDoc(jsonObj);
    QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);

//    sendTextMessage(jsonString);
}

void StartRelayTrigger::getSystemPage(QWebSocket *webSender)
{
    int dateTimeMethod;
    if (ntp){
        dateTimeMethod = 1;
    }else{
        dateTimeMethod = 2;
    }

    QString message = QString("{\"menuID\":\"system\", \"SwVersion\":\"%1\", \"HwVersion\":\"%2\", \"dateTimeMethod\":\"%3\", \"ntpServer\":\"%4\", \"location\":\"%5\"}")
            .arg(SWVERSION).arg(HWVERSION).arg(dateTimeMethod).arg(ntpServer).arg(timeLocation);
    webSender->sendTextMessage(message);
}
void StartRelayTrigger::setLocation(QString location)
{
    if (!location.contains("Select")){
        QString command = QString("ln -sf /usr/share/zoneinfo/%1  /etc/localtime").arg(location);
        system(command.toStdString().c_str());
        timeLocation = location;
    }
}
void StartRelayTrigger::getTimezone()
{
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    QString Timezone;
    QProcess getTimezoneQProcess;
    arguments << "-c" << QString("ls -la /etc/localtime | grep '/usr/share/zoneinfo/' | awk '{print $11}'");
    getTimezoneQProcess.start(prog , arguments);
    getTimezoneQProcess.waitForFinished(100);
    Timezone = QString(getTimezoneQProcess.readAll()).trimmed();
    arguments.clear();
    qDebug() << "Timezone" << Timezone;
    timeLocation = Timezone.replace("/usr/share/zoneinfo/","");
}

void StartRelayTrigger::updateNTPServer(QString value)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("NETWORK/ntpServer",value);
        settings->setValue("NETWORK/ntp","true");
        ConnmanETH0->setNTPServer(value);
        ntpServer=value;
        system("timedatectl set-ntp 1");
    }
    ntp = true;
}

void StartRelayTrigger::setcurrentDatetime(QString dateTime)
{
    QString date;
    QString time;
    if (dateTime.split(" ").size() >= 2){
        date = QString(dateTime.split(" ").at(0)).replace("/","-");
        time = QString(dateTime.split(" ").at(1))+":00";
        system("timedatectl set-ntp 0");
        QString command = QString("date --set '%1 %2'").arg(date).arg(time);
        system(command.toStdString().c_str());
//        ConnmanETH0->setNTPServer("");
    }

    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("NETWORK/ntp","false");
    }
    ntp = false;
}




void StartRelayTrigger::scanFileUpdate()
{
    QStringList fileupdate;
    fileupdate = findFile();
    if (fileupdate.size() > 0){
        if(foundfileupdate == false)
            updateFirmware();
    }
}

QStringList StartRelayTrigger::findFile()
{
    QStringList listfilename;
    QString ss="/usr/share/apache2/default-site/htdocs/uploads/";
    const char *sss ;
    sss = ss.toStdString().c_str();
    QDir dir1("/usr/share/apache2/default-site/htdocs/uploads/");
    QString filepath;
    QString filename;
    QFileInfoList fi1List( dir1.entryInfoList( QDir::Files, QDir::Name) );
    foreach( const QFileInfo & fi1, fi1List ) {
        filepath = QString::fromUtf8(fi1.absoluteFilePath().toLocal8Bit());
        filename = QString::fromUtf8(fi1.fileName().toLocal8Bit());
        listfilename << filepath;
        qDebug() << filepath;// << filepath.toUtf8().toHex();
    }
    return listfilename;
}

void StartRelayTrigger::updateFirmware()
{
    qDebug() << "Start updateFirmware";
    foundfileupdate = true;
    QStringList fileupdate;
    fileupdate = findFile();
    system("mkdir -p /tmp/update");
    if (fileupdate.size() > 0){
//        updateScreenOnOff(true);
        qDebug() << "Start update";
        updateStatus = 1;
        QString sendMessage = QString("{\"menuID\":\"update\", \"updateStatus\":%1}").arg(updateStatus);
        QString commandCopyFile = "cp " + QString(fileupdate.at(0)) + " /tmp/update/update.tar";
        system(commandCopyFile.toStdString().c_str());
        system("tar -xf /tmp/update/update.tar -C /tmp/update/");
        system("sh /tmp/update/update.sh");
        system("rm -rf /tmp/update");
        system("rm -rf /usr/share/apache2/default-site/htdocs/uploads/*");
        updateStatus = 2;
        sendMessage = QString("{\"menuID\":\"update\", \"updateStatus\":%1}").arg(updateStatus);
        qDebug() << "Update complete";
//        exit(0);
    }
    foundfileupdate = false;
}
