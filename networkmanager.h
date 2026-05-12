#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QComboBox>
#include <QLabel>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    void ipInit(QComboBox *comboBox);

private:
    QLabel *labListen;                  // 状态栏标签
    QTcpServer *tcpServer;              // TCP服务器
    QTcpSocket *tcpSocket = nullptr;    // TCP通信的socket

signals:
};

#endif // NETWORKMANAGER_H
