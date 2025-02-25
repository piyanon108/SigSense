#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <QObject>
#include <QList>
#include <QByteArray>
#include "QWebSocketServer"
#include "QWebSocket"
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>

#include <QObject>

class ChatServer : public QObject
{
    Q_OBJECT
public:
    explicit ChatServer(quint16 port, QObject *parent = nullptr);
    void sendtoWiFiManager(QString message);
    void sendtoWebClient(QString message);
    void sendtoClient(QString message);
    int clientNum = 0;
    QWebSocket *WiFiManagerSocketClients;


public Q_SLOTS:
    void broadcastMessage(QString message);

signals:
    void onNumClientChanged(int numclient);
    void newMessage(QWebSocket *pSender,QJsonObject messageObj, QString message);
    void keymasterPress(QString message);
    void onNewClient(QWebSocket *pSender);

private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QList<QWebSocket *> m_WebSocketClients;
    QList<QWebSocket *> m_SocketClients;

    QWebSocket *newSocket;


private Q_SLOTS:
    void onNewConnection();
    void commandProcess(QString message, QWebSocket *pSender);
    void processMessage(QString message);
    void socketDisconnected();
};

#endif // CHATSERVER_H
