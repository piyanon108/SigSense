#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <QObject>
#include <QDebug>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>

class SocketClient : public QObject
{
    Q_OBJECT
public:
    explicit SocketClient(QObject *parent = Q_NULLPTR);
    void createConnection(int channelIdInRole, QString ipaddress, quint16 port);
    bool isConnected = false;
    QWebSocket m_webSocket;

Q_SIGNALS:
    void closed(int channelIdInRole, QString ipaddress);
    void newMessage(QWebSocket *pSender,QJsonObject messageObj, QString message);
    void TextMessageReceived(QString message,int channelIdInRole, QString ipaddress);
    void connected();

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(QString message);
    void onError(QAbstractSocket::SocketError error);

private:

    QUrl m_url;
    QString m_ipaddress;
    int m_socketID;
    bool m_debug = false;
};

#endif // SOCKETCLIENT_H
