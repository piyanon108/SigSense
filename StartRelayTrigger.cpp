#include "StartRelayTrigger.h"

StartRelayTrigger::StartRelayTrigger(QObject *parent)
    : QObject{parent}
{    
    gpioKeyEvent = new GPIOKeyEvent;
    outputTrigger = new OutputTrigger;
    reconnect = new QTimer;
    resetTestmode = new QTimer;
    ConnmanETH0 = new NetworkConnman("eth0");
    connect(gpioKeyEvent, SIGNAL(keypress(quint32)), this, SLOT(keypress(quint32)));
    myDatabase = new Database("plcanalog","z7020","Ifz8zean6868**","127.0.0.1",this);
    gpioKeyEvent->start();

    readFileSetting();

    slaveSocket = new SocketClient();
    slaveSocket->createConnection(0,adcRemoteAddress,adcRemotePort);

    plcServerSocket = new SocketClient();
    plcServerSocket->createConnection(0,plcServerAddress,plcServerPort);

    socketServer = new ChatServer(adcRemotePort);
    websocketServer = new ChatServer(8009);
    adc_file_watcher = new QFileSystemWatcher(this);

    clearOld(ADCPATH);
    adc_file_watcher->addPath(ADCPATH);

    connect(websocketServer, SIGNAL(newMessage(QWebSocket*,QJsonObject,QString)),this,SLOT(newMessage(QWebSocket*,QJsonObject,QString)));
    connect(websocketServer, SIGNAL(onNewClient(QWebSocket*)),this,SLOT(onNewClient(QWebSocket*)));

    connect(socketServer, SIGNAL(keymasterPress(QString)),this,SLOT(keymasterPress(QString)));
    connect(reconnect, SIGNAL(timeout()), this, SLOT(checkConnection()));

    connect(plcServerSocket, SIGNAL(connected()), this, SLOT(plcServerSocketConnected()));
    connect(plcServerSocket, SIGNAL(newMessage(QWebSocket*,QJsonObject,QString)),this,SLOT(newMessage(QWebSocket*,QJsonObject,QString)));

    connect(adc_file_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
    connect(adc_file_watcher,SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));
    connect(resetTestmode,SIGNAL(timeout()), this, SLOT(resettestmode()));

    reconnect->start(1000);



    connect(myDatabase,SIGNAL(alertToWebClient(QString)),websocketServer,SLOT(broadcastMessage(QString)));


}

void StartRelayTrigger::updatePLCInputDelay(QWebSocket *pSender)
{
    for (int i=1; i<=8; i++){
        QJsonObject jsonObj;
        jsonObj[QString("plc_input_%1").arg(i)] =  int(plcInputDelay[i-1]);
        // Convert to JSON string (optional)
        QJsonDocument jsonDoc(jsonObj);
        QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);
        pSender->sendTextMessage(jsonString);
    }
}

void StartRelayTrigger::onNewClient(QWebSocket *pSender)
{
    QJsonObject jsonObj;
    jsonObj["objectName"] = "SwVersion";
    jsonObj["SwVersion"] = SWVERSION;
    jsonObj["HwVersion"] = HWVERSION;
    jsonObj["HwName"] = "FPGA";

    jsonObj["master_start_relay_offset_usec"] = int(master_start_relay_offset_usec);
    jsonObj["slave_start_relay_offset_usec"] = int(slave_start_relay_offset_usec);
    jsonObj["pulse_relay_period_offset_usec"] = int(pulse_relay_period_offset_usec);
    jsonObj["adc_write_offset_usec"] = int(adc_write_offset_usec);
    jsonObj["adc_write_period_usec"] = int(adc_write_period_usec);
    jsonObj["surgeA_threshold"] = int(surgeA_threshold);
    jsonObj["surgeB_threshold"] = int(surgeB_threshold);
    jsonObj["surgeC_threshold"] = int(surgeC_threshold);
    jsonObj["adcRemoteAddress"] = QString("%1:%2").arg(adcRemoteAddress).arg(adcRemotePort);
    jsonObj["plcServerAddress"] = QString("%1:%2").arg(plcServerAddress).arg(plcServerPort);
    jsonObj["cableLength"] = double(cableLength);
    jsonObj["velocity_factor"] = double(velocity_factor);
    for (int i=1; i<=8; i++){
        jsonObj[QString("plc_input_%1").arg(i)] =  int(plcInputDelay[i-1]);
    }

    // Convert to JSON string (optional)
    QJsonDocument jsonDoc(jsonObj);
    QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);

    // Print JSON output
    qDebug() << jsonString;
    pSender->sendTextMessage(jsonString);
//    updatePLCInputDelay(pSender);
    updateNetworkInfo(eth0,eth0.phyNetworkName);
}
double StartRelayTrigger::calculatePosition(double deltaT, double vfac)
{
    const double c = 3.0e8*vfac; // ความเร็วแสงใน m/s
    double x = (cableLength - (c * deltaT)) / 2.0;
    return x;
}
void StartRelayTrigger::resettestmode()
{
    testMode = "Surge";
    resetTestmode->stop();
}
void StartRelayTrigger::surgeEventDetect(QJsonObject jsonObjSurge, bool local)
{
    QJsonObject jsonObjSurgeLocal;
    QJsonObject jsonObjSurgeRemote;

    jsonObjSurgeLocal["time"] = "";
    jsonObjSurgeRemote["time"] = "";

    int adc_channel = jsonObjSurge["adc_channel"].toInt();
    if (local)
    {
        switch (adc_channel) {
        case 0:
            jsonObjSurgeA_Local = jsonObjSurge;
            break;
        case 1:
            jsonObjSurgeB_Local = jsonObjSurge;
            break;
        case 2:
            jsonObjSurgeC_Local = jsonObjSurge;
            break;
        default:
            break;
        }
    }
    else
    {
        switch (adc_channel) {
        case 0:
            jsonObjSurgeA_Remote = jsonObjSurge;
            break;
        case 1:
            jsonObjSurgeB_Remote = jsonObjSurge;
            break;
        case 2:
            jsonObjSurgeC_Remote = jsonObjSurge;
            break;
        default:
            break;
        }
    }

    switch (adc_channel)
    {
        case 0:
            jsonObjSurgeLocal = jsonObjSurgeA_Local;
            jsonObjSurgeRemote = jsonObjSurgeA_Remote;
            break;
        case 1:
            jsonObjSurgeLocal = jsonObjSurgeB_Local;
            jsonObjSurgeRemote = jsonObjSurgeB_Remote;
            break;
        case 2:
            jsonObjSurgeLocal = jsonObjSurgeC_Local;
            jsonObjSurgeRemote = jsonObjSurgeC_Remote;
            break;
        default:
            break;
    }
    if (jsonObjSurgeLocal["time"] == jsonObjSurgeRemote["time"] )
    {
        int deltaT = jsonObjSurgeLocal["nanosec"].toInt() - jsonObjSurgeRemote["nanosec"].toInt();
        QString phaseChannel = adc_channel==0 ? "A" : adc_channel==1 ? "B" : "C";
        qDebug() << "surgeEventDetect Channel:" << phaseChannel << "Local Time:" << QString("%1 %2.%3").arg(jsonObjSurgeLocal["date"].toString()).arg(jsonObjSurgeLocal["time"].toString()).arg(jsonObjSurgeLocal["nanosec"].toInt()) << "Remote Time:" << QString("%1 %2.%3").arg(jsonObjSurgeRemote["date"].toString()).arg(jsonObjSurgeRemote["time"].toString()).arg(jsonObjSurgeRemote["nanosec"].toInt()) << "delta_t=" << deltaT << "nanosecond";

        double positionA = calculatePosition(deltaT/1e9,velocity_factor);
        double positionB = cableLength - positionA;

        if ((positionA <= cableLength) & (positionB <= cableLength)){
            qDebug() << "Position from Local: " << positionA << " meters";
            qDebug() << "Position from Remote: " << positionB << " meters";
            QJsonObject jsonObj;
            jsonObj["objectName"] = "surgeEventDetect";
            jsonObj["PositionFromLocal"] = positionA;
            jsonObj["PositionFromRemote"] = positionB;
            jsonObj["time"] = jsonObjSurgeLocal["time"];
            jsonObj["time"] = jsonObjSurgeLocal["date"];
            jsonObj["local_nanosec"] = jsonObjSurgeLocal["nanosec"];
            jsonObj["remote_nanosec"] = jsonObjSurgeRemote["nanosec"];
            jsonObj["delta_nanosec"] = deltaT;
            jsonObj["local_url"] = jsonObjSurgeLocal["url"];
            jsonObj["remote_url"] = jsonObjSurgeRemote["url"];
//            qDebug() << jsonObj;

            if (plcServerSocket->isConnected == true)
            {
                QJsonDocument jsonDoc(jsonObj);
                QJsonDocument jsonObjSurgeLocalDoc(jsonObjSurgeLocal);
                QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);
                QString jsonObjSurgeLocalString = jsonObjSurgeLocalDoc.toJson(QJsonDocument::Compact);

                plcServerSocket->m_webSocket.sendTextMessage(jsonObjSurgeLocalString);
                plcServerSocket->m_webSocket.sendTextMessage(jsonString);

                qDebug() << jsonObjSurgeLocal;
                qDebug() << jsonObjSurgeRemote;
            }
        }
    }
}

void StartRelayTrigger::plcServerSocketConnected()
{
    QJsonObject jsonObj;
    jsonObj["objectName"] = "SwVersion";
    jsonObj["SwVersion"] = SWVERSION;
    jsonObj["HwVersion"] = HWVERSION;
    jsonObj["HwName"] = "FPGA";

    jsonObj["master_start_relay_offset_usec"] = int(master_start_relay_offset_usec);
    jsonObj["slave_start_relay_offset_usec"] = int(slave_start_relay_offset_usec);
    jsonObj["pulse_relay_period_offset_usec"] = int(pulse_relay_period_offset_usec);
    jsonObj["adc_write_offset_usec"] = int(adc_write_offset_usec);
    jsonObj["adc_write_period_usec"] = int(adc_write_period_usec);
    jsonObj["surgeA_threshold"] = int(surgeA_threshold);
    jsonObj["surgeB_threshold"] = int(surgeB_threshold);
    jsonObj["surgeC_threshold"] = int(surgeC_threshold);
    jsonObj["adcRemoteAddress"] = QString("%1:%2").arg(adcRemoteAddress).arg(adcRemotePort);
    jsonObj["plcServerAddress"] = QString("%1:%2").arg(plcServerAddress).arg(plcServerPort);
    jsonObj["cableLength"] = double(cableLength);
    jsonObj["velocity_factor"] = double(velocity_factor);

    // Convert to JSON string (optional)
    QJsonDocument jsonDoc(jsonObj);
    QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);

    // Print JSON output
    qDebug() << jsonString;

    plcServerSocket->m_webSocket.sendTextMessage(jsonString);
}
void StartRelayTrigger::keypress(quint32 nanosec)
{
    QString formattedDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    qDebug() << "keypress" <<  formattedDateTime << nanosec;
    testMode = "RelayTest";
    QJsonObject jsonObj;
    jsonObj["objectName"] = "startRelay";
    jsonObj["DateTime"] = formattedDateTime;
    jsonObj["testMode"] = testMode;
    QJsonDocument jsonDoc(jsonObj);
    QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);
    slaveSocket->m_webSocket.sendTextMessage(jsonString);

    QString currentDatetime = getChrrentDateTime();
    QJsonObject plcJsonObj;
    int nanoSec = currentDatetime.split(".").at(1).toInt();
    plcJsonObj["objectName"] = "eventRecord";
    plcJsonObj["event"] = "Send Master Start";
    plcJsonObj["date"] = QDate::currentDate().toString("yyyyMMdd");
    plcJsonObj["time"] = QTime::currentTime().toString("hhmmss");;
    plcJsonObj["nanosec"] = nanoSec;
    plcJsonObj["timestamp"] = currentDatetime;
    plcJsonObj["testMode"] = testMode;


    // Convert to JSON string (optional)
    QJsonDocument plcJsonDoc(plcJsonObj);
    QString plcJsonString = plcJsonDoc.toJson(QJsonDocument::Compact);


    if (plcServerSocket->isConnected == true){
        plcServerSocket->m_webSocket.sendTextMessage(plcJsonString);
        qDebug() << "plcServerSocket->m_webSocket.sendTextMessage(jsonString)" << plcJsonString;
    }
}
void StartRelayTrigger::onDirectoryChanged(const QString &path)
{
//    qDebug() << "Directory changed: " << path;
    bool clear = false;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd/hhmmss");
    QString targetDir = QString("/usr/share/apache2/default-site/htdocs/adc/%1/").arg(date);
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "Directory does not exist!";
        return;
    }

    // Get all files in the directory (excluding directories)
    QStringList files = dir.entryList(QDir::Files);

    // Print out the filenames
    foreach (const QString &file, files) {
//        qDebug() << file;
        fileManager(path+file,targetDir,date,clear);
    }
}
void StartRelayTrigger::clearOld(const QString &path)
{
//    qDebug() << "Directory changed: " << path;
    bool clear = true;
    QString oldDir = QDateTime::currentDateTime().toString("yyyyMMdd/tmp/hhmmss");
    QString targetDir = QString("/usr/share/apache2/default-site/htdocs/adc/%1/").arg(oldDir);
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "Directory does not exist!";
        return;
    }

    // Get all files in the directory (excluding directories)
    QStringList files = dir.entryList(QDir::Files);

    // Print out the filenames
    foreach (const QString &file, files) {
//        qDebug() << file;
        fileManager(path+file,targetDir,oldDir,clear);
    }
}
void StartRelayTrigger::fileManager(QString sourceFile,  QString targetDir, QString date,bool clear = false)
{
    QDir dir(targetDir);
    if (!dir.exists()) {
        if (dir.mkpath(".")) {
//            qDebug() << "Target directory created!";
        } else {
            qDebug() << "Failed to create target directory!";
            return ;
        }
    }

    // Construct the target file path
    QString targetFile = targetDir + QFileInfo(sourceFile).fileName();

    // Move the file
    if (QFile::rename(sourceFile, targetFile))
    {
//        qDebug() << "File moved successfully!";
        if (clear == false)
            newADCDataRecord(targetFile,QFileInfo(sourceFile).fileName(),date);
    }
    else
    {
        qDebug() << "Failed to move the file!";
    }
}
quint32 StartRelayTrigger::get10Nanosec(int adcNum)
{
    quint32 timestamp = 0;
    if (testMode.contains("Surge"))
    {
        switch (adcNum) {
        case 0:
            timestamp = readRegister(REG_SURGE_TIMESTAMP0);
            break;
        case 1:
            timestamp = readRegister(REG_SURGE_TIMESTAMP1);
            break;
        case 2:
            timestamp = readRegister(REG_SURGE_TIMESTAMP2);
            break;
        default:
            break;
        }
        return timestamp;
    }
    else
    {
        switch (adcNum) {
        case 0:
            timestamp = readRegister(REG_MANUAL_TIMESTAMP0);
            break;
        case 1:
            timestamp = readRegister(REG_MANUAL_TIMESTAMP1);
            break;
        case 2:
            timestamp = readRegister(REG_MANUAL_TIMESTAMP2);
            break;
        default:
            break;
        }
        return timestamp;
    }
}
void StartRelayTrigger::newADCDataRecord(const QString &path, const QString &name, const QString &dir)
{
    QStringList adc_filename = name.split("_");
    QString date_yyyyMMdd = "";
    QString time_hhmmss = "";

    int sec_epoch1970_Timestamp = 0;
    int adc_ch = -1;
    if (adc_filename.size() == 3)
    {
        date_yyyyMMdd = adc_filename.at(0);
        time_hhmmss =  adc_filename.at(1);
//        sec_epoch1970_Timestamp = QString(adc_filename.at(2)).toInt();
        adc_ch = adc_filename.at(2) == "data0.raw" ? 0 : adc_filename.at(2) == "data1.raw" ? 1 : 2;
    }
    QString url = QString("http://%1/adc/%2/%3").arg(eth0.ipaddress).arg(dir).arg(name);
    QJsonObject jsonObj;
    int nanoSec = int(get10Nanosec(adc_ch))*10;
    jsonObj["objectName"] = "adcRawData";
    jsonObj["date"] = date_yyyyMMdd;
    jsonObj["time"] = time_hhmmss;
    jsonObj["nanosec"] = nanoSec;
    jsonObj["timestamp"] = QString("%1 %2.%3").arg(date_yyyyMMdd).arg(time_hhmmss).arg(nanoSec);//int(sec_epoch1970_Timestamp);
    jsonObj["adc_channel"] = adc_ch;
    jsonObj["url"] = url;
    jsonObj["filename"] = name;
    jsonObj["testMode"] = testMode;

    // Convert to JSON string (optional)
    QJsonDocument jsonDoc(jsonObj);
    QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);

    // Print JSON output
//    qDebug() << "Channel_Local:" << (adc_ch) << " Start Mode:" << testMode << QString("%1 %2.%3").arg(date_yyyyMMdd).arg(time_hhmmss).arg(nanoSec) << url << jsonString;
    if (testMode == "Surge")
    {
        surgeEventDetect(jsonObj,true);
        if (slaveSocket->isConnected == true)
            slaveSocket->m_webSocket.sendTextMessage(jsonString);
    }
    else {
        if (plcServerSocket->isConnected == true)
            plcServerSocket->m_webSocket.sendTextMessage(jsonString);
        qDebug() << "Channel_Local:" << (adc_ch) << " Start Mode:" << testMode << QString("%1 %2.%3").arg(date_yyyyMMdd).arg(time_hhmmss).arg(nanoSec) << url << jsonString;
    }
    resetTestmode->start(2000);
}
void StartRelayTrigger::onFileChanged(const QString &path)
{
    qDebug() << "File changed: " << path;
    // Check if the directory exists
}

void StartRelayTrigger::newMessage(QWebSocket *pSender, QJsonObject messageObj, QString message)
{
    if (pSender == &(plcServerSocket->m_webSocket))
    {
        if (messageObj["objectName"] == "SwVersion")
        {
            qDebug() << "message" << message;
        }
        else if ((messageObj["objectName"] == "PatternTest")||(messageObj["objectName"] == "ManualTest")||(messageObj["objectName"] == "RelayTest")||(messageObj["objectName"] == "SurgeTest")||(messageObj["objectName"] == "Periodic"))
        {
            testMode = messageObj["objectName"].toString();
            QString formattedDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
            qDebug() << "newMessage plcServerSocket" <<  formattedDateTime << message;
            slaveSocket->m_webSocket.sendTextMessage(formattedDateTime);
            slaveSocket->m_webSocket.sendTextMessage(message);
            keyLocalPress("newMessage plcServerSocket");

            QString currentDatetime = getChrrentDateTime();
            QJsonObject jsonObj;
            int nanoSec = currentDatetime.split(".").at(1).toInt();
            jsonObj["objectName"] = "eventRecord";
            jsonObj["event"] = "Send Master Start";
            jsonObj["date"] = QDate::currentDate().toString("yyyyMMdd");
            jsonObj["time"] = QTime::currentTime().toString("hhmmss");;
            jsonObj["nanosec"] = nanoSec;
            jsonObj["timestamp"] = currentDatetime;
            jsonObj["testMode"] = testMode;


            // Convert to JSON string (optional)
            QJsonDocument jsonDoc(jsonObj);
            QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);


            if (plcServerSocket->isConnected == true){
                plcServerSocket->m_webSocket.sendTextMessage(jsonString);
                qDebug() << "plcServerSocket->m_webSocket.sendTextMessage(jsonString)" << jsonString;
            }
        }
        else if (QString(messageObj["objectName"].toString()).contains("plc_input"))
        {
             int index = QString(messageObj["objectName"].toString()).replace("plc_input_","").toInt();
             plcInputDelay[index-1] = messageObj["value"].toInt();
        }
        else
        {
            qDebug() << "message" << message;
        }
    }
    else if (pSender == &(slaveSocket->m_webSocket))
    {

    }
    else
    {
        qDebug() << "message from web" << message;
        if (QString(messageObj["objectName"].toString()).contains("plc_input"))
        {
            if (plcServerSocket->isConnected){
                plcServerSocket->m_webSocket.sendTextMessage(message);
                int index = QString(messageObj["objectName"].toString()).replace("plc_input_","").toInt();
                plcInputDelay[index-1] = messageObj["value"].toInt();
            }


        }
        else if (messageObj["objectName"] == "updateLocalNetwork")
        {
            uint8_t dhcpmethod =  messageObj["dhcpmethod"].toString() == "on";
            QString ipaddress =  messageObj["ipaddress"].toString();
            QString subnet =  messageObj["subnet"].toString();
            QString gateway =  messageObj["gateway"].toString();
            QString pridns =  messageObj["pridns"].toString();
            QString secdns =  messageObj["secdns"].toString();
            QString phyNetworkName = QJsonValue(messageObj["phyNetworkName"]).toString();
            qDebug() << "updateLocalNetwork" << phyNetworkName << dhcpmethod << ipaddress << subnet << gateway << pridns << secdns;
            updateNetwork(dhcpmethod, ipaddress, subnet, gateway, pridns, secdns, phyNetworkName);
        }
        else if(messageObj["objectName"] == "master_start_relay_offset_usec")
        {
            updateSetting("ADC", messageObj["objectName"].toString(), messageObj["value"].toInt());
            master_start_relay_offset_usec = messageObj["value"].toInt();
            writeRegister(REG_DELAY_START,master_start_relay_offset_usec);
        }
        else if(messageObj["objectName"] == "slave_start_relay_offset_usec")
        {
            updateSetting("ADC", messageObj["objectName"].toString(), messageObj["value"].toInt());
            slave_start_relay_offset_usec = messageObj["value"].toInt();
        }
        else if(messageObj["objectName"] == "pulse_relay_period_offset_usec")
        {
            updateSetting("ADC", messageObj["objectName"].toString(), messageObj["value"].toInt());
            pulse_relay_period_offset_usec = messageObj["value"].toInt();
            writeRegister(REG_PULSE_PERIOD_OFFSET,pulse_relay_period_offset_usec);
        }
        else if(messageObj["objectName"] == "network_latency_usec")
        {
            updateSetting("ADC", messageObj["objectName"].toString(), messageObj["value"].toInt());
            network_latency_usec = messageObj["value"].toInt();
        }
        else if(messageObj["objectName"] == "adc_write_offset_usec")
        {
            updateSetting("ADC", messageObj["objectName"].toString(), messageObj["value"].toInt());
            adc_write_offset_usec = messageObj["value"].toInt();
            writeRegister(REG_ADC_READ_OFFSET,adc_write_offset_usec);
        }
        else if(messageObj["objectName"] == "adc_write_period_usec")
        {
            updateSetting("ADC", messageObj["objectName"].toString(), messageObj["value"].toInt());
            adc_write_period_usec = messageObj["value"].toInt();
        }
        else if(messageObj["objectName"] == "velocity_factor")
        {
            updateSetting("ADC", messageObj["objectName"].toString(), messageObj["value"].toInt());
            velocity_factor = messageObj["value"].toDouble();
        }
        else if(messageObj["objectName"] == "cableLength")
        {
            updateSetting("ADC", messageObj["objectName"].toString(), messageObj["value"].toInt());
            cableLength = messageObj["value"].toDouble();
        }
        else if(messageObj["objectName"] == "surgeA_threshold")
        {
            surgeA_threshold = messageObj["value"].toInt();            
            updateSetting("ADC", messageObj["objectName"].toString(), surgeA_threshold);
            writeRegister(REG_SURGE_TRIGGER0,(surgeA_threshold*adc_res)/adc_vref);
        }
        else if(messageObj["objectName"] == "surgeB_threshold")
        {
            surgeB_threshold = messageObj["value"].toInt();
            updateSetting("ADC", messageObj["objectName"].toString(), surgeB_threshold);
            writeRegister(REG_SURGE_TRIGGER1,(surgeB_threshold*adc_res)/adc_vref);
        }
        else if(messageObj["objectName"] == "surgeC_threshold")
        {
            surgeC_threshold = messageObj["value"].toInt();
            updateSetting("ADC", messageObj["objectName"].toString(), surgeC_threshold);
            writeRegister(REG_SURGE_TRIGGER2,(surgeC_threshold*adc_res)/adc_vref);
        }
        else if(messageObj["objectName"] == "plcServerAddress")
        {
            QString plcServerAddressAndPort = messageObj["value"].toString();
            if (plcServerAddressAndPort.contains(":"))
            {
                QStringList plcServerAddressAndPortList = plcServerAddressAndPort.split(":");
                if (plcServerAddressAndPortList.size() == 2)
                {
                    plcServerAddress = plcServerAddressAndPortList.at(0);
                    plcServerPort = QString(plcServerAddressAndPortList.at(0)).toInt();
                }
            }
            updateSetting("ADC", "plcServerAddress", plcServerAddress);
            updateSetting("ADC", "plcServerPort", plcServerPort);

            if (plcServerSocket->isConnected)
                plcServerSocket->m_webSocket.close();
            plcServerSocket->createConnection(0,plcServerAddress,plcServerPort);
        }
        else if(messageObj["objectName"] == "adcRemoteAddress")
        {
            QString adcRemoteAddressAndPort = messageObj["value"].toString();
            if (adcRemoteAddressAndPort.contains(":"))
            {
                QStringList adcRemoteAddressAndPortList = adcRemoteAddressAndPort.split(":");
                if (adcRemoteAddressAndPortList.size() == 2)
                {
                    adcRemoteAddress = adcRemoteAddressAndPortList.at(0);
                    adcRemotePort = QString(adcRemoteAddressAndPortList.at(0)).toInt();
                }
            }
            updateSetting("ADC", "adcRemoteAddress", adcRemoteAddress);
            updateSetting("ADC", "adcRemotePort", adcRemotePort);

            if (slaveSocket->isConnected)
                slaveSocket->m_webSocket.close();
            slaveSocket->createConnection(0,adcRemoteAddress,adcRemotePort);
        }
        else if (messageObj["menuID"] == "getSystemPage")
        {
            getSystemPage(pSender);
        }
        else if (messageObj["menuID"] == "getNetworkPage")
        {

        }
        else if ((messageObj["menuID"] == "resetapp") || (messageObj["menuID"] == "rebootSystem"))
        {
            exit(0);
        }
        else if (messageObj["menuID"] == "updateNTPServer")
        {
            QString value = QJsonValue(messageObj["ntpServer"]).toString();
            updateNTPServer(value);
        }
        else if (messageObj["menuID"] == "updateTime")
        {
            QString value = QJsonValue(messageObj["dateTime"]).toString();
            setcurrentDatetime(value);
        }
        else if (messageObj["menuID"] == "setLocation"){
            QString value = QJsonValue(messageObj["location"]).toString();
            setLocation(value);
        }
        else if (messageObj["menuID"] == "systembackup"){

        }
        else if (messageObj["menuID"] == "systemrestore")
        {

        }
        else if (messageObj["menuID"] == "updateFirmware")
        {
            updateFirmware();
        }
        else if (messageObj["menuID"] == "insertNewUserInDatabase")
        {
            QString username =  QJsonValue(messageObj["username"]).toString();
            QString password =  QJsonValue(messageObj["password"]).toString();
            int userlevel =  QJsonValue(messageObj["userlevel"]).toInt();
            myDatabase->createUser(username,password,userlevel);
        }
        else if (messageObj["menuID"] == "applyEditUser")
        {
            QString username =  QJsonValue(messageObj["username"]).toString();
            QString password =  QJsonValue(messageObj["password"]).toString();
            int userlevel =  QJsonValue(messageObj["userlevel"]).toInt();
            qDebug() << "changePassword" << messageObj["menuID"] << username << password << userlevel;
            myDatabase->changePassword(username,password,userlevel);
        }
        else if (messageObj["menuID"] == "deleteUser")
        {
            int userID =  QJsonValue(messageObj["userID"]).toInt();
            myDatabase->deleteUser(userID);
//            deleteUser(userID);
        }
    }
}


void StartRelayTrigger::checkConnection()
{
    if (slaveSocket->isConnected == false)
        slaveSocket->createConnection(0,adcRemoteAddress,adcRemotePort);
    if (plcServerSocket->isConnected == false)
    {
        plcServerSocket->createConnection(0,plcServerAddress,plcServerPort);
    }
    else
    {
        QJsonDocument jsonDoc;
        QJsonObject Param;
        Param.insert("objectName","keepAlive");
        Param.insert("HwName","FPGA");
        jsonDoc.setObject(Param);
        QString raw_data = QJsonDocument(Param).toJson(QJsonDocument::Compact).toStdString().c_str();
        plcServerSocket->m_webSocket.sendTextMessage(raw_data);
    }
}

// Function to Read a 32-bit value to the register
quint32 StartRelayTrigger::readRegister(uint32_t register_address)
{
    int fd;
    quint32 read_result = -1;
    uint64_t target = register_address;
    void *map_base, *virt_addr;
    unsigned offset;

    // Get the system's page size (dynamically determined)
    unsigned int page_size = sysconf(_SC_PAGESIZE);  // Get the actual page size
    unsigned int map_size = page_size;  // Size of the memory mapping

    // Open /dev/mem with read/write permissions
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        qCritical() << "Failed to open /dev/mem";
        return read_result;  // Handle error gracefully
    }

    // Calculate the offset within the page
    offset = (unsigned int)(target & (page_size - 1));

    // Map the physical memory address to virtual memory
    map_base = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~(page_size - 1));
    if (map_base == MAP_FAILED) {
        qCritical() << "Memory mapping failed";
        close(fd);  // Don't forget to close the file descriptor
        return read_result;
    }

    // Calculate the virtual address to write to
    virt_addr = map_base + offset;

    // Read back the value to verify the write operation
    read_result = *((volatile uint32_t *)virt_addr);

    // Output debug information
//    qDebug() << "readRegister"
//             << QString("0x%1").arg(register_address, 0, 16)  // Display register address in hex
//             << "Read Value:" << read_result
//             << "Page Size:" << page_size
//             << "Offset:" << offset;

    // Clean up: unmap and close the file descriptor
    if (munmap(map_base, map_size) == -1) {
        qCritical() << "Memory unmapping failed";
    }

    close(fd);  // Close the file descriptor when done
    return read_result;
}

// Function to write a 32-bit value to the register
void StartRelayTrigger::writeRegister(uint32_t register_address, uint32_t value)
{
    int fd;
    uint32_t read_result = -1;
    uint64_t target = register_address;
    void *map_base, *virt_addr;
    unsigned offset;

    // Get the system's page size (dynamically determined)
    unsigned int page_size = sysconf(_SC_PAGESIZE);  // Get the actual page size
    unsigned int map_size = page_size;  // Size of the memory mapping

    // Open /dev/mem with read/write permissions
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        qCritical() << "Failed to open /dev/mem";
        return;  // Handle error gracefully
    }

    // Calculate the offset within the page
    offset = (unsigned int)(target & (page_size - 1));

    // Map the physical memory address to virtual memory
    map_base = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~(page_size - 1));
    if (map_base == MAP_FAILED) {
        qCritical() << "Memory mapping failed";
        close(fd);  // Don't forget to close the file descriptor
        return;
    }

    // Calculate the virtual address to write to
    virt_addr = map_base + offset;

    // Write the 32-bit value to the register
    *((volatile uint32_t *)virt_addr) = value;

    // Read back the value to verify the write operation
    read_result = *((volatile uint32_t *)virt_addr);

    // Output debug information
    qDebug() << "writeRegister"
             << QString("0x%1").arg(register_address, 0, 16)  // Display register address in hex
             << "Read Value:" << read_result
             << "Page Size:" << page_size
             << "Offset:" << offset;

    // Clean up: unmap and close the file descriptor
    if (munmap(map_base, map_size) == -1) {
        qCritical() << "Memory unmapping failed";
    }

    close(fd);  // Close the file descriptor when done
}

QString StartRelayTrigger::getChrrentDateTime()
{
    // Get current system time in nanoseconds
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    // Extract seconds and nanoseconds
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration) - seconds;

    // Convert to time_t for formatting
    std::time_t time_now = seconds.count();
    std::tm tm_now = *std::localtime(&time_now);

    // Use stringstream for formatted output
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y%m%d %H%M%S")
        << "." << std::setw(9) << std::setfill('0') << nanoseconds.count();

    return QString::fromStdString(oss.str());
}

void StartRelayTrigger::keymasterPress(QString message)
{
    QJsonDocument d = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject messageObj = d.object();
    qDebug() << "StartRelayTrigger::keymasterPress" << message;
    if (messageObj["objectName"] == "startRelay"){
        writeRegister(REG_DELAY_START,slave_start_relay_offset_usec);
        outputTrigger->keymasterPress(message);
        testMode =  QJsonValue(messageObj["testMode"]).toString();
        QString currentDatetime = getChrrentDateTime();
        QJsonObject jsonObj;
        int nanoSec = currentDatetime.split(".").at(1).toInt();
        jsonObj["objectName"] = "eventRecord";
        jsonObj["event"] = "Receive Master Start";
        jsonObj["date"] = QDate::currentDate().toString("yyyyMMdd");
        jsonObj["time"] = QTime::currentTime().toString("hhmmss");;
        jsonObj["nanosec"] = nanoSec;
        jsonObj["timestamp"] = currentDatetime;
        jsonObj["testMode"] = testMode;


        // Convert to JSON string (optional)
        QJsonDocument jsonDoc(jsonObj);
        QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);


        if (plcServerSocket->isConnected == true){
            plcServerSocket->m_webSocket.sendTextMessage(jsonString);
            qDebug() << "plcServerSocket->m_webSocket.sendTextMessage(jsonString)" << jsonString;
        }


        QThread::msleep(100);
        writeRegister(REG_DELAY_START,master_start_relay_offset_usec);
    }
    else if ((messageObj["objectName"] == "PatternTest")||(messageObj["objectName"] == "ManualTest")||(messageObj["objectName"] == "RelayTest")||(messageObj["objectName"] == "SurgeTest") || (messageObj["objectName"] == "Periodic"))
    {
        writeRegister(REG_DELAY_START,slave_start_relay_offset_usec);
        outputTrigger->keymasterPress(message);
        testMode =  QJsonValue(messageObj["objectName"]).toString();

        QString currentDatetime = getChrrentDateTime();
        QJsonObject jsonObj;
        int nanoSec = currentDatetime.split(".").at(1).toInt();
        jsonObj["objectName"] = "eventRecord";
        jsonObj["event"] = "Receive Master Start";
        jsonObj["date"] = QDate::currentDate().toString("yyyyMMdd");
        jsonObj["time"] = QTime::currentTime().toString("hhmmss");;
        jsonObj["nanosec"] = nanoSec;
        jsonObj["timestamp"] = currentDatetime;
        jsonObj["testMode"] = testMode;


        // Convert to JSON string (optional)
        QJsonDocument jsonDoc(jsonObj);
        QString jsonString = jsonDoc.toJson(QJsonDocument::Compact);


        if (plcServerSocket->isConnected == true){
            plcServerSocket->m_webSocket.sendTextMessage(jsonString);
            qDebug() << "plcServerSocket->m_webSocket.sendTextMessage(jsonString)" << jsonString;
        }

        QThread::msleep(100);
        writeRegister(REG_DELAY_START,master_start_relay_offset_usec);
    }
    else if(messageObj["objectName"] == "adcRawData")
    {
        surgeEventDetect(messageObj,false);
//        qDebug() << QString("Channel_Remote: %1  Start Mode: %2 %3 %4.%5").arg(messageObj["adc_channel"].toInt()).arg(messageObj["testMode"].toString()).arg(messageObj["date"].toString()).arg(messageObj["time"].toString()).arg(messageObj["nanosec"].toInt()) << message;
//        if (plcServerSocket->isConnected){
//            plcServerSocket->m_webSocket.sendTextMessage(message);
////            qDebug() << "message from another adc";
//        }
    }
}

void StartRelayTrigger::keyLocalPress(QString message)
{
    writeRegister(REG_DELAY_START,master_start_relay_offset_usec);
    outputTrigger->keymasterPress(message);
}

