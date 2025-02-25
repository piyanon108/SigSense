#include "ChatServer.h"

#include <QDateTime>

QT_USE_NAMESPACE

ChatServer::ChatServer(quint16 port, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(Q_NULLPTR),
    m_clients()
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Chat Server"),
                                              QWebSocketServer::NonSecureMode,
                                              this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, port))
    {
        qDebug() << "Chat Server listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &ChatServer::onNewConnection);
    }
}
void ChatServer::sendtoWebClient(QString message)
{
    Q_FOREACH (QWebSocket *pClient, m_WebSocketClients)
    {
        pClient->sendTextMessage(message);
    }
}
void ChatServer::sendtoWiFiManager(QString message)
{
      WiFiManagerSocketClients->sendTextMessage(message);
}
void ChatServer::sendtoClient(QString message)
{
    Q_FOREACH (QWebSocket *pClient, m_SocketClients)
    {
        pClient->sendTextMessage(message);
    }
}

void ChatServer::commandProcess(QString message, QWebSocket *pSender){
    QJsonDocument d = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject messageObj = d.object();
    QString getCommand =  QJsonValue(messageObj["menuID"]).toString();
    emit newMessage(pSender, messageObj, message);
}

void ChatServer::processMessage(QString message)
{
//    qDebug() << message;
    emit keymasterPress(message);
    QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());
    commandProcess(message, pSender);

}
void ChatServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (WiFiManagerSocketClients == pClient) WiFiManagerSocketClients = Q_NULLPTR;
    if (pClient)
    {
        m_clients.removeAll(pClient);
        m_SocketClients.removeAll(pClient);
        m_WebSocketClients.removeAll(pClient);
        pClient->deleteLater();
        qDebug() << pClient->localAddress().toString() << "has disconect";
    }
    clientNum = m_clients.length();
    if (clientNum <= 0)
    {
        emit onNumClientChanged(clientNum);
    }
}
void ChatServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &ChatServer::processMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &ChatServer::socketDisconnected);
    m_clients << pSocket;
    qDebug() << "On New Connection from address : " << pSocket->peerName();
    emit onNewClient(pSocket);
    if (clientNum <= 0)
    {
        clientNum = m_clients.length();
        emit onNumClientChanged(clientNum);

    }
    else {
        clientNum = m_clients.length();
    }

}

void ChatServer::broadcastMessage(QString message){
    Q_FOREACH (QWebSocket *pClient, m_clients)
    {
        pClient->sendTextMessage(message);
    }
}
