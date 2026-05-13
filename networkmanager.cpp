#include "networkmanager.h"

#include <QNetworkInterface>
#include <QNetworkProxy>

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    tcpServer_ = new QTcpServer(this);
    tcpSocket_ = new QTcpSocket(this);
    tcpClient_ = new QTcpSocket(this);
    udpSocket_ = new QUdpSocket(this);
    // 强制关闭代理
    tcpClient_->setProxy(QNetworkProxy::NoProxy);

    slotsInit();
}

void NetworkManager::slotsInit()
{
    connect(tcpServer_,&QTcpServer::newConnection,this,&NetworkManager::do_newConnection);

    connect(tcpClient_,&QTcpSocket::stateChanged,this,&NetworkManager::sgn_stateChange);
    connect(tcpClient_,&QTcpSocket::readyRead,this,[=](){
        sgn_readyRead(tcpClient_->readAll());
    });
    connect(tcpClient_,&QTcpSocket::errorOccurred,this,[=](QAbstractSocket::SocketError){
        isOpen_ = false;
        curNetworkModel_ = None;
        tcpClient_->abort();
        QString errMsg = tcpClient_->errorString();
        emit sgn_showMessage("TCP连接失败: " + errMsg);
        emit sgn_btnStateChanged(false);
    });

    connect(udpSocket_,&QUdpSocket::stateChanged,this,&NetworkManager::sgn_stateChange);
    connect(udpSocket_,&QUdpSocket::readyRead,this,&NetworkManager::do_socketReadyRead);
}

void NetworkManager::ipInit(QComboBox *comboBox)
{
    // 先加入本地回环地址
    comboBox->addItem("127.0.0.1");
    // 返回本地全部网卡对象列表
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interface, interfaceList){
        // flags是组合的标志，必须加 testFlag 单独判断
        if(!interface.flags().testFlag(QNetworkInterface::IsUp))
            continue;
        if(interface.flags().testFlag(QNetworkInterface::IsLoopBack))
            continue;
        if(interface.type() == QNetworkInterface::Virtual)
            continue;

        foreach(QNetworkAddressEntry addr, interface.addressEntries()){
            if(addr.ip().protocol() == QAbstractSocket::IPv4Protocol){
                comboBox->addItem(addr.ip().toString());
            }
        }
    }
}

void NetworkManager::closeConnection()
{
    isOpen_ = false;
    curNetworkModel_ = None;

    if (tcpSocket_) {
        tcpSocket_->close();
    }
    if (tcpServer_ && tcpServer_->isListening()) {
        tcpServer_->close();
    }
    if (tcpClient_) {
        tcpClient_->abort();
    }
    udpSocket_->abort();
    emit sgn_btnStateChanged(false);
}

void NetworkManager::sendData(const QByteArray &content)
{
    if(curNetworkModel_ == TcpServer && tcpSocket_ &&
        tcpSocket_->state() == QAbstractSocket::ConnectedState){
            tcpSocket_->write(content);
    }
    else if(curNetworkModel_ == TcpClient && tcpClient_->state() == QAbstractSocket::ConnectedState){
        tcpClient_->write(content);
    }
    else if(curNetworkModel_ == UDP && udpSocket_->state() == QAbstractSocket::ConnectedState){
        QHostAddress targetAddr(udpTargetIP_);
        udpSocket_->writeDatagram(content, targetAddr, udpTargetPort_);
    }
}

void NetworkManager::do_btnOpenClose(CurNetworkModel networkModel, QString ip, quint16 port)
{
    if(!isOpen_){
        curNetworkModel_ = networkModel;
        if(curNetworkModel_ == TcpServer){
            QHostAddress address(ip);
            if (!tcpServer_->listen(address, port)) {
                emit sgn_showMessage("服务器启动失败: " + tcpServer_->errorString());
                return;
            }
        }
        else if(curNetworkModel_ == TcpClient){
            tcpClient_->connectToHost(ip, port);
            // 如果是已经连接失败的情况下，按钮不会打开
            if(tcpClient_->state() != QAbstractSocket::ConnectingState ||
                tcpClient_->state() != QAbstractSocket::ConnectedState ||
                tcpClient_->state() != QAbstractSocket::HostLookupState){
                return;
            }
        }
        else if(curNetworkModel_ == UDP){
            // UDP 绑定的时候不写 IP，自动绑定本机所有网卡IP，只需端口号一致。先保存目标ip和端口
            udpTargetIP_ = ip;
            udpTargetPort_ = port;
            if (!udpSocket_->bind(port)){
                emit sgn_showMessage("UDP连接失败: " + udpSocket_->errorString());
                return;
            }
            ui->textEdit->appendPlainText("绑定端口："+QString::number(udpSocket_->localPort()));
        }
        isOpen_ = true;
        emit sgn_btnStateChanged(isOpen_);
    }
    else{
        // 关闭服务端
        if (tcpServer_->isListening()){
            if (tcpSocket_ && tcpSocket_->state() == QAbstractSocket::ConnectedState) {
                tcpSocket_->disconnectFromHost();
            }
            tcpServer_->close();
        }
        // 关闭客户端
        if (tcpClient_->state() == QAbstractSocket::ConnectedState ||
            tcpClient_->state() == QAbstractSocket::ConnectingState ||
            tcpClient_->state() == QAbstractSocket::HostLookupState) {
            tcpClient_->abort();
        }
        if(curNetworkModel_ == UDP)
            udpSocket_->abort();
        isOpen_ = false;
        curNetworkModel_ = None;
        emit sgn_btnStateChanged(false);
    }
}

void NetworkManager::do_newConnection()
{
    // 更新新的socket，只支持一个客户端
    tcpSocket_ = tcpServer_->nextPendingConnection();

    // 主动发送当前状态（连接后才得到的socket）
    emit sgn_stateChange(tcpSocket_->state());

    connect(tcpSocket_,&QTcpSocket::stateChanged,this,&NetworkManager::sgn_stateChange);
    connect(tcpSocket_,&QTcpSocket::disconnected,this,[=](){
        tcpSocket_->deleteLater();
        tcpSocket_ = nullptr;
    });
    connect(tcpSocket_,&QTcpSocket::readyRead,this,[=](){
        sgn_readyRead(tcpSocket_->readAll());
    });
}

void NetworkManager::do_socketReadyRead()
{
    while(udpSocket_->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocket_->pendingDatagramSize());
        QHostAddress peerAddr;
        quint16 peerPort;
        udpSocket_->readDatagram(datagram.data(),datagram.size(),&peerAddr,&peerPort);
        QString str = datagram.data();
        // QString peer = "[From "+peerAddr.toString()+":"+QString::number(peerPort)+"] ";
        sgn_readyRead(str.toUtf8());
    }
}
