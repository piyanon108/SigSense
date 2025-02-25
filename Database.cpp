#include "Database.h"

Database::Database(QString dbName, QString user, QString Password, QString host,QObject *parent) :
    QObject(parent)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(Password);
    database_createConnection();
    _dbName = dbName;
}

void Database::restartMysql()
{
    system("systemctl stop mysql");
    system("systemctl start mysql");

    qDebug() << "Restart MySQL";
}

bool Database::database_createConnection()
{
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
        return false;
    }
    db.close();
    qDebug() << "Database connected";

    return true;
}
void Database::getActiveClientInDatabase(int userID,int roleID, int deviceID)
{
    int deviceIndex = 0;
    int deviceIdInRole = 0;
    int roleIndex = 0;
    QString name = "";
    QString ipAddress = "";
    QString roleName = "";
    QString type = "";
    int subType = 0;
    QString databaseUser = "";
    QString databaseName = "";
    QString databasePassword = "";
    int socketPort = 0;
    QString description = "";
    QString serialnumber = "";
    int templateID = 0;
    int radioIoPort = 0;
    QString query = QString("SELECT role.deviceID, role.roleName, role.userID, role.roleID, role.roleID, role.deviceIdInRole, role.id AS roleIndex, "
                            "deviceList.id AS deviceIndex, deviceList.Name, deviceList.templateID, deviceList.ipaddress, deviceList.Username, deviceList.Password,  deviceList.radioIoPort, deviceList.serialnumber, "
                            "deviceTemplate.iconName, deviceTemplate.type, deviceTemplate.subType, deviceTemplate.databaseUser , deviceTemplate.databaseName , deviceTemplate.databasePassword, deviceTemplate.socketPort, deviceTemplate.description "
                            "FROM deviceList INNER JOIN deviceTemplate ON deviceTemplate.id=deviceList.templateID INNER JOIN role ON role.deviceID = deviceList.id WHERE role.roleID=%1 AND deviceList.id=%2 ORDER BY role.deviceIdInRole").arg(roleID).arg(deviceID);
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }else
    {
        while (qry.next()) {
            roleIndex = qry.value("roleIndex").toInt();
            deviceIdInRole = qry.value("deviceIdInRole").toInt();
            deviceIndex = qry.value("deviceIndex").toInt();
            name = qry.value("Name").toString();
            ipAddress = qry.value("ipaddress").toString();
            roleName = qry.value("roleName").toString();
            roleID= qry.value("roleID").toInt();
            userID= qry.value("userID").toInt();
            type = qry.value("type").toString();
            subType = qry.value("subType").toInt();
            databaseUser = qry.value("databaseUser").toString();
            databaseName = qry.value("databaseName").toString();
            databasePassword = qry.value("databasePassword").toString();
            socketPort = qry.value("socketPort").toInt();
            description = qry.value("description").toString();
            templateID = qry.value("templateID").toInt();
            radioIoPort = qry.value("radioIoPort").toInt();
            serialnumber = qry.value("serialnumber").toString();
            emit appendNewActiveClient(roleIndex, deviceIndex, deviceIdInRole, name, ipAddress, roleID , roleName, userID, type, subType, databaseUser, databaseName, databasePassword, uint16_t(socketPort), description, templateID,radioIoPort,serialnumber);
        }
    }
    db.close();
}
void Database::getActiveClientInDatabase(int userID,int roleID)
{
    int deviceIndex = 0;
    int deviceIdInRole = 0;
    int roleIndex = 0;
    QString name = "";
    QString ipAddress = "";
    QString roleName = "";
    QString type = "";
    int subType = 0;
    QString databaseUser = "";
    QString databaseName = "";
    QString databasePassword = "";
    int socketPort = 0;
    QString description = "";
    int templateID = 0;
    int radioIoPort = 0;
    QString query = QString("SELECT role.deviceID, role.roleName, role.userID, role.roleID, role.roleID, role.deviceIdInRole, role.id AS roleIndex, "
                            "deviceList.id AS deviceIndex, deviceList.Name, deviceList.templateID, deviceList.ipaddress, deviceList.Username, deviceList.Password,  deviceList.radioIoPort, "
                            "deviceTemplate.iconName, deviceTemplate.type, deviceTemplate.subType, deviceTemplate.databaseUser , deviceTemplate.databaseName , deviceTemplate.databasePassword, deviceTemplate.socketPort, deviceTemplate.description "
                            "FROM deviceList INNER JOIN deviceTemplate ON deviceTemplate.id=deviceList.templateID INNER JOIN role ON role.deviceID = deviceList.id WHERE role.userID=%1 AND role.roleID=%2 ORDER BY role.deviceIdInRole").arg(userID).arg(roleID);
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }else
    {
        while (qry.next()) {
            roleIndex = qry.value("roleIndex").toInt();
            deviceIdInRole = qry.value("deviceIdInRole").toInt();
            deviceIndex = qry.value("deviceIndex").toInt();
            name = qry.value("Name").toString();
            ipAddress = qry.value("ipaddress").toString();
            roleName = qry.value("roleName").toString();
            roleID= qry.value("roleID").toInt();
            userID= qry.value("userID").toInt();
            type = qry.value("type").toString();
            subType = qry.value("subType").toInt();
            databaseUser = qry.value("databaseUser").toString();
            databaseName = qry.value("databaseName").toString();
            databasePassword = qry.value("databasePassword").toString();
            socketPort = qry.value("socketPort").toInt();
            description = qry.value("description").toString();
            templateID = qry.value("templateID").toInt();
            radioIoPort = qry.value("radioIoPort").toInt();
            emit appendNewActiveClient(roleIndex, deviceIndex, deviceIdInRole, name, ipAddress, roleID , roleName, userID, type, subType, databaseUser, databaseName, databasePassword, uint16_t(socketPort), description, templateID,radioIoPort);
        }
    }
    db.close();
}
void Database::getActiveClientInDatabase()
{
    int deviceIndex = 0;
    int deviceIdInRole = 0;
    int roleIndex = 0;
    QString name = "";
    QString ipAddress = "";
    QString roleName = "";
    QString type = "";
    int subType = 0;
    QString databaseUser = "";
    QString databaseName = "";
    QString databasePassword = "";
    int socketPort = 0;
    QString description = "";
    int templateID = 0;
    int radioIoPort = 0;
    int roleID = 0;
    int userID = 0;
    QString query = QString("SELECT role.deviceID, role.roleName, role.userID, role.roleID, role.roleID, role.deviceIdInRole, role.id AS roleIndex, "
                            "deviceList.id AS deviceIndex, deviceList.Name, deviceList.templateID, deviceList.ipaddress, deviceList.Username, deviceList.Password,  deviceList.radioIoPort, "
                            "deviceTemplate.iconName, deviceTemplate.type, deviceTemplate.subType, deviceTemplate.databaseUser , deviceTemplate.databaseName , deviceTemplate.databasePassword, deviceTemplate.socketPort, deviceTemplate.description "
                            "FROM deviceList INNER JOIN deviceTemplate ON deviceTemplate.id=deviceList.templateID INNER JOIN role ON role.deviceID = deviceList.id ORDER BY role.deviceIdInRole");
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }else
    {
        while (qry.next()) {
            roleIndex = qry.value("roleIndex").toInt();
            deviceIdInRole = qry.value("deviceIdInRole").toInt();
            deviceIndex = qry.value("deviceIndex").toInt();
            name = qry.value("Name").toString();
            ipAddress = qry.value("ipaddress").toString();
            roleName = qry.value("roleName").toString();
            roleID= qry.value("roleID").toInt();
            userID= qry.value("userID").toInt();
            type = qry.value("type").toString();
            subType = qry.value("subType").toInt();
            databaseUser = qry.value("databaseUser").toString();
            databaseName = qry.value("databaseName").toString();
            databasePassword = qry.value("databasePassword").toString();
            socketPort = qry.value("socketPort").toInt();
            description = qry.value("description").toString();
            templateID = qry.value("templateID").toInt();
            radioIoPort = qry.value("radioIoPort").toInt();
            emit appendNewActiveClient(roleIndex, deviceIndex, deviceIdInRole, name, ipAddress, roleID , roleName, userID, type, subType, databaseUser, databaseName, databasePassword, uint16_t(socketPort), description, templateID,radioIoPort);
        }
    }
    db.close();
}
void Database::getAllClientInDatabase()
{
    int id = 0;
    QString name = "";
    QString ipAddress = "";
    QString type = "";
    int subType = 0;
    QString databaseUser = "";
    QString databaseName = "";
    QString databasePassword = "";
    int socketPort = 0;
    QString description = "";
    int templateID = 0;
    QString query = QString("SELECT deviceList.id, deviceList.Name, deviceList.templateID, deviceList.ipaddress, "
                            "deviceTemplate.type, deviceTemplate.subType, deviceTemplate.databaseUser, deviceTemplate.databasePassword, deviceTemplate.databaseName, deviceTemplate.socketPort, deviceTemplate.description "
                            "FROM deviceList INNER JOIN deviceTemplate ON deviceTemplate.id=deviceList.templateID ORDER BY id ASC");
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }else
    {
        while (qry.next()) {
            id = qry.value("id").toInt();
            name = qry.value("Name").toString();
            ipAddress = qry.value("ipaddress").toString();
            type = qry.value("type").toString();
            subType = qry.value("subType").toInt();
            databaseUser = qry.value("databaseUser").toString();
            databaseName = qry.value("databaseName").toString();
            databasePassword = qry.value("databasePassword").toString();
            socketPort = qry.value("socketPort").toInt();
            description = qry.value("description").toString();
            templateID = qry.value("templateID").toInt();
            emit appendNewClient(id, name, ipAddress, type, subType, databaseUser, databaseName, databasePassword, uint16_t(socketPort), description, templateID);
        }
    }
//    currentUserID = userID;
    db.close();
}
int Database::getDeviceIdInRole(int roleID)
{
    int deviceIdInRole = 0;
    QString query = QString("SELECT deviceIdInRole FROM role WHERE roleID=%1 ORDER BY deviceIdInRole DESC LIMIT 1").arg(roleID);
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }else
    {
        while (qry.next())
        {
            deviceIdInRole = qry.value("deviceIdInRole").toInt();
        }
    }
    deviceIdInRole++;
 db.close();
 return  deviceIdInRole;
}

int Database::getNewRoleID()
{
    int roleID = 0;
    QString query = QString("SELECT roleID FROM role ORDER BY roleID DESC LIMIT 1");
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }else
    {
        while (qry.next())
        {
            roleID = qry.value("roleID").toInt();
        }
    }
    roleID++;
 db.close();
 return  roleID;
}

void Database::deleteRoleID(int roleID)
{
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QString query = QString("DELETE FROM role WHERE roleID=%1").arg(roleID);
    qDebug() << "deleteRoleID" << query;
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }
    db.close();
}

void Database::changeRoleName(int roleID, QString roleName)
{
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QSqlQuery qry;
    qry.prepare("UPDATE role SET roleName = :roleName "
                "WHERE roleID = :roleID");
    qry.addBindValue(roleName);
    qry.addBindValue(roleID);
    if (!qry.exec()){
        qWarning() << "c++: ERROR! "  << qry.lastError();
    }
    db.close();
}
bool Database::checkSameDiviceInrole(int roleID, int deviceID)
{
    int _roleID = 0;
    int _deviceD = 0;
    QString query = QString("SELECT roleID, deviceID FROM role WHERE roleID=%1 AND deviceID=%2 LIMIT 1").arg(roleID).arg(deviceID);
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }else
    {
        while (qry.next())
        {
            _roleID = qry.value("roleID").toInt();
            _deviceD = qry.value("deviceID").toInt();
        }
    }
 db.close();
 return ((_roleID == roleID) & (deviceID == _deviceD));
}
int Database::insertNewDeviceInRole(int roleID, QString roleName,int deviceID, int userID)
{
    if (checkSameDiviceInrole(roleID,deviceID)) return roleID;
    int deviceIdInRole = getDeviceIdInRole(roleID);
    if (roleID == 0) roleID=getNewRoleID();

    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return roleID;
    }
    QSqlQuery qry;
    qry.prepare("INSERT INTO role (roleID , deviceID , roleName , userID, deviceIdInRole) "
                  "VALUES (:roleID , :deviceID , :roleName , :userID , :deviceIdInRole)");
    qry.bindValue(":roleID", roleID);
    qry.bindValue(":deviceID", deviceID);
    qry.bindValue(":roleName", roleName);
    qry.bindValue(":userID", userID);
    qry.bindValue(":deviceIdInRole", deviceIdInRole);
    if (!qry.exec()){
        qWarning() << "c++: ERROR! "  << qry.lastError();
    }
    db.close();
    return roleID;
}
void Database::getNewClientInDatabase(int userID)
{
    int id = 0;
    QString name = "";
    QString ipAddress = "";
    QString type = "";
    int subType = 0;
    QString databaseUser = "";
    QString databaseName = "";
    QString databasePassword = "";
    int socketPort = 0;
    QString description = "";
    int templateID = 0;
    QString query = QString("SELECT deviceList.id, deviceList.Name, deviceList.templateID, deviceList.ipaddress, "
                            "deviceTemplate.type, deviceTemplate.subType, deviceTemplate.databaseUser, deviceTemplate.databasePassword, deviceTemplate.databaseName, deviceTemplate.socketPort, deviceTemplate.description "
                            "FROM deviceList INNER JOIN deviceTemplate ON deviceTemplate.id=deviceList.templateID WHERE userID=%1 ORDER BY ID DESC LIMIT 1").arg(userID);
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }else
    {
        while (qry.next()) {
            id = qry.value("id").toInt();
            name = qry.value("Name").toString();
            ipAddress = qry.value("ipaddress").toString();
            type = qry.value("type").toString();
            subType = qry.value("subType").toInt();
            databaseUser = qry.value("databaseUser").toString();
            databaseName = qry.value("databaseName").toString();
            databasePassword = qry.value("databasePassword").toString();
            socketPort = qry.value("socketPort").toInt();
            description = qry.value("description").toString();
            templateID = qry.value("templateID").toInt();
            emit appendNewClient(id, name, ipAddress, type, subType, databaseUser, databaseName, databasePassword, uint16_t(socketPort), description, templateID);
        }
    }
//    currentRoleName = roleName;
    db.close();
}

void Database::deleteClient(int id)
{
    QString query = QString("DELETE FROM deviceList WHERE id=%1").arg(id);
    if (!db.open()) {
        qDebug() << "database error! database can not open.";
        emit mysqlError();
        restartMysql();
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }
    db.close();
}

void Database::insertClientInDatabase(int userID, int templateID, QString Name, QString ipAddress, QString Username, QString Password, int radioIoPort, QString serialnumber)
{
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QSqlQuery qry;
    qry.prepare("INSERT INTO deviceList (Name , templateID , ipaddress , userID, Username, Password, radioIoPort, serialnumber) "
                  "VALUES (:Name, :templateID, :ipAddress, :userID, :Username, :Password, :radioIoPort, :serialnumber)");
    qry.bindValue(":Name", Name);
    qry.bindValue(":templateID", templateID);
    qry.bindValue(":ipAddress", ipAddress);
    qry.bindValue(":userID", userID);
    qry.bindValue(":Username", Username);
    qry.bindValue(":Password", Password);
    qry.bindValue(":radioIoPort", radioIoPort);
    qry.bindValue(":serialnumber", serialnumber);

    if (!qry.exec()){
        qWarning() << "c++: ERROR! "  << qry.lastError();
    }
    db.close();

}

void Database::updateClientInDatabase(int userID, int templateID, QString Name, QString ipAddress, int id, QString Username, QString Password, int radioIoPort, QString serialnumber)
{
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QSqlQuery qry;
    qry.prepare("UPDATE deviceList SET Name = :Name , templateID = :templateID , ipaddress = :ipaddress , Username = :Username , Password = :Password , radioIoPort = :radioIoPort , serialnumber = :serialnumber "
                "WHERE id = :id");
    qry.addBindValue(Name);
    qry.addBindValue(templateID);
    qry.addBindValue(ipAddress);    
    qry.addBindValue(Username);
    qry.addBindValue(Password);
    qry.addBindValue(radioIoPort);
    qry.addBindValue(serialnumber);
    qry.addBindValue(id);
    qDebug() << id << "updateClientInDatabase" << qry.executedQuery() << Name << templateID << ipAddress << userID << Username << Password;
    if (!qry.exec()){
        qWarning() << "c++: ERROR! "  << qry.lastError();
    }
    db.close();
}

void Database::insertDataLogger(int txIndex, double fwdPowerWatt, double fwdPowerDB, double maxPowerWatt, double rwdPowerWatt, double rwdPowerDB, double vswr, bool connectionStatus, QString stationName, int frequency, int duration, QString startLog, QString endLog)
{
    QString query = QString("INSERT INTO datalogger (txIndex , fwdPowerWatt , fwdPowerDB , maxPowerWatt , rwdPowerWatt , rwdPowerDB , vswr , connectionStatus , stationName , frequency , duration , startLog , endLog) "
                            "VALUES (%1, %2, %3, %4, %5, %6, %7, %8, '%9', %10, %11, '%12', '%13')"
                            ).arg(txIndex).arg(fwdPowerWatt).arg(fwdPowerDB).arg(maxPowerWatt).arg(rwdPowerWatt).arg(rwdPowerDB).arg(vswr).arg(connectionStatus).arg(stationName).arg(frequency).arg(duration).arg(startLog).arg(endLog);
    qDebug() << query;
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qWarning() << "c++: ERROR! "  << qry.lastError();
    }
    db.close();
}

void Database::changeDeviceInRole(int roleindex, int newDeviceID)
{
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QSqlQuery qry;
    qry.prepare("UPDATE role SET deviceID = :newDeviceID "
                "WHERE id = :roleindex");
    qry.addBindValue(newDeviceID);
    qry.addBindValue(roleindex);
    qDebug() << "changeDeviceInRole" << qry.executedQuery() << roleindex << newDeviceID;
    if (!qry.exec()){
        qWarning() << "c++: ERROR! "  << qry.lastError();
    }
    db.close();
}

void Database::deleteDeviceIdInRole(int roleindex)
{
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QString query = QString("DELETE FROM role WHERE id=%1").arg(roleindex);
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }
    db.close();
}

//User management
void Database::createUser(QString user, QString Password, uint8_t userLevel)
{
    QString query = QString("INSERT INTO member (Username, Password, userlevel) VALUE ('%1', CONCAT('*', UPPER(SHA1(UNHEX(SHA1('%2'))))), %3)").arg(user).arg(Password).arg(userLevel);
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QSqlQuery qry;
//    qDebug() << query;
    qry.prepare(query);
    if (!qry.exec()){
        qWarning() << "c++: ERROR! "  << qry.lastError();
        QString message = QString("{\"menuID\":\"oncreateUser\",\"Username\":\"%1\",\"completed\":\"%2\"}").arg(user).arg("false");
        emit alertToWebClient(message);
    }
    else {
        QString message = QString("{\"menuID\":\"oncreateUser\",\"Username\":\"%1\",\"completed\":\"%2\"}").arg(user).arg("true");
        emit alertToWebClient(message);
    }
    db.close();
}

void Database::changePassword(QString user, QString newPassword, int userlevel)
{
    QString query = "";
    if (newPassword != "")
        query = QString("UPDATE member SET Password=CONCAT('*', UPPER(SHA1(UNHEX(SHA1('%1'))))), userlevel=%2 WHERE Username='%3'").arg(newPassword).arg(userlevel).arg(user);
    else
        query = QString("UPDATE member SET userlevel=%1 WHERE Username='%2'").arg(userlevel).arg(user);
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qWarning() << "c++: ERROR! "  << qry.lastError();
        QString message = QString("{\"menuID\":\"onchangePassword\",\"Username\":\"%1\",\"completed\":\"%2\"}").arg(user).arg("false");
        emit alertToWebClient(message);
    }
    else {
        QString message = QString("{\"menuID\":\"onchangePassword\",\"Username\":\"%1\",\"completed\":\"%2\"}").arg(user).arg("true");
        emit alertToWebClient(message);
    }
    db.close();
}

void Database::deleteUser(int userID)
{
    if (!db.open())
    {
        qWarning() << "c++: ERROR! "  << "database error! database can not open.";
        emit mysqlError();
        return;
    }
    QString query = QString("DELETE FROM member WHERE id=%1").arg(userID);
    QSqlQuery qry;
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
        QString message = QString("{\"menuID\":\"onDeleteUser\",\"userID\":%1,\"completed\":\"%2\"}").arg(userID).arg("false");
        emit alertToWebClient(message);
    }
    else {
        QString message = QString("{\"menuID\":\"onDeleteUser\",\"userID\":%1,\"completed\":\"%2\"}").arg(userID).arg("true");
        emit alertToWebClient(message);
    }
    query = QString("DELETE FROM role WHERE userID=%1").arg(userID);
    qry.prepare(query);
    if (!qry.exec()){
        qDebug() << qry.lastError();
    }
    db.close();
}


void Database::dumpDatabase()
{
    system("rm -f /var/www/html/database/rcms_backup*.bin");
    system("rm -f /tmp/database/rcms_backup*.bin");
    system("sync");
    system("mkdir -p /var/www/html/database");
    system("mkdir -p /tmp/database");
    system("chown -R www-data:www-data /var/www/html/database");
    system("rm /var/www/html/database/rcms_backup*.bin");
    QString commandRestorSQL = QString("mysqldump -uroot -pOTL324$ %1 > /tmp/database/rcms_backup.bin").arg(_dbName);
    system(commandRestorSQL.toStdString().c_str());
    QString fileName = QString("/var/www/html/database/rcms_backup%1.bin").arg(QDateTime::currentDateTime().toSecsSinceEpoch());
    QString tarCommand = QString("tar -cf %1 /tmp/database/rcms_backup.bin").arg(fileName);
    qDebug() << "tarCommand" << tarCommand;
    system(tarCommand.toStdString().c_str());
    system("chown -R www-data:www-data /var/www/html/database/*");
    system("sync");
    emit backupcompleted(fileName);
}

void Database::restoreDatabase()
{
    system("systemctl restart apache2");
    system("tar -xf /var/www/html/uploads/database.tar -C /");
    QString commandRestorSQL = QString("mysql -uroot -pOTL324$ %1 < /tmp/database/rcms_backup.bin").arg(_dbName);
    system(commandRestorSQL.toStdString().c_str());
    system("rm -f /var/www/html/database/*");
    system("rm -f /var/www/html/uploads/database.tar");
    system("sync");
    emit restorecompleted();
}
