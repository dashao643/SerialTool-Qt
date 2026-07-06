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

  void ipInit(QComboBox *comboBox);
  void closeConnection();
  void sendData(const QByteArray &content);

public slots:
  void do_btnOpenClose(CurNetworkModel networkModel, QString ip, quint16 port);
  void do_btnOpenClose(quint16 localport, QString targetIp, quint16 targetPort);

signals:
  void sgn_btnStateChanged(bool isOpen);
  void sgn_stateChange(QAbstractSocket::SocketState state);
  void sgn_readyRead(const QByteArray &byteArray);
  void sgn_labelShowState(const QString& text);
  void sgn_showMessage(const QString& text);

private:
  bool isOpen_ = false;
  CurNetworkModel curNetworkModel_ = None;
  QTcpServer *tcpServer_;              // TCP服务器
  QTcpSocket *tcpSocket_;              // TCP服务器对应socket
  QTcpSocket *tcpClient_;              // TCP客户端
  QUdpSocket *udpSocket_;

  QString udpTargetIP_;
  quint16 udpTargetPort_;

  void slotsInit();

private slots:
  void do_newConnection();
  void do_socketReadyRead();
};

#endif // NETWORKMANAGER_H
