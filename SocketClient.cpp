#include "SocketClient.h"

QT_USE_NAMESPACE

//! [constructor]
SocketClient::SocketClient(QObject *parent) :
    QObject(parent)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &SocketClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &SocketClient::onDisconnected);
    connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
    [=](QAbstractSocket::SocketError error){
        if (isConnected){
            emit closed(m_socketID, m_ipaddress);
            isConnected = false;
        }
        qDebug() << "Connecting Error: ";
    });
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &SocketClient::onTextMessageReceived);
}
//! [constructor]

//! [createConnection]
void SocketClient::createConnection(int channelIdInRole, QString ipaddress, quint16 port)
{
    QString url = QString("ws://%1:%2").arg(ipaddress).arg(port);
    QUrl iGateSocketServerUrl(url);
    if (m_debug)
        qDebug() << "WebSocket server:" << url;
    m_url = iGateSocketServerUrl;
    m_ipaddress = ipaddress;
    m_socketID = channelIdInRole;
    m_webSocket.open(QUrl(iGateSocketServerUrl));
}
//! [createConnection]

//! [onConnected]
void SocketClient::onConnected()
{
    isConnected = true;
    if (m_debug)
        qDebug() << "WebSocket connected";
    emit connected();
//    m_webSocket.sendTextMessage(QStringLiteral("Hello, world!"));
}
//! [onConnected]

//! [onTextMessageReceived]
void SocketClient::onTextMessageReceived(QString message)
{
//    emit TextMessageReceived(message, m_socketID, m_ipaddress);
    QJsonDocument d = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject messageObj = d.object();
    emit newMessage(&m_webSocket, messageObj, message);
}
//! [onTextMessageReceived]

void SocketClient::onDisconnected()
{
    isConnected = false;
    qDebug() << m_ipaddress << "WebSocket disconnected";
    emit closed(m_socketID, m_ipaddress);
}

void SocketClient::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "Connecting Error: ";
}
