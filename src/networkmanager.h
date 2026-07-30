#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QComboBox>

typedef enum {
    None = -1,
    TcpServer,
    TcpClient,
    UDP
}CurNetworkModel;

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    void loadLocalIP(QComboBox *comboBox);
    void closeConnection();
    void sendData(const QByteArray &content);
    int getClientCnt();
    
public slots:
    void do_btnOpenClose(CurNetworkModel networkModel, QString ip, quint16 port);
    void do_btnOpenClose(quint16 localport, QString targetIp, quint16 targetPort);

signals:
    void sgn_btnStateChanged(bool isOpen);
    void sgn_stateChange(QAbstractSocket::SocketState state);            // tcp客户端, udp状态信号
    // void sgn_tcpServerStateChange(QAbstractSocket::SocketState state);   // tcp服务端状态信号
    void sgn_readyRead(const QByteArray &byteArray);
    void sgn_labelShowState(const QString& text);
    void sgn_showMessage(const QString& text);

private:
    void slotsInit();
    void clearTcpSocketList();

private slots:
    void do_newConnection();
    void do_socketReadyRead();

private:
    bool isOpen_ = false;
    CurNetworkModel curNetworkModel_ = None;
    QTcpServer *tcpServer_;              // TCP服务器
    QList<QTcpSocket*> tcpSocketList_;   // TCP服务器对应socket列表
    QTcpSocket *tcpClient_;              // TCP客户端
    QUdpSocket *udpSocket_;

    QString udpTargetIP_;
    quint16 udpTargetPort_;
};

#endif // NETWORKMANAGER_H
