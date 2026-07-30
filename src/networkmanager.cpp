#include "networkmanager.h"

#include <QNetworkInterface>
#include <QNetworkProxy>

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    tcpSocketList_.clear();

    tcpServer_ = new QTcpServer(this);
    tcpClient_ = new QTcpSocket(this);
    udpSocket_ = new QUdpSocket(this);

    // 强制关闭代理
    tcpClient_->setProxy(QNetworkProxy::NoProxy);
    udpSocket_->setProxy(QNetworkProxy::NoProxy);

    slotsInit();
}

NetworkManager::~NetworkManager()
{
    foreach(QTcpSocket *tcpSocket, tcpSocketList_) {
        if(tcpSocket != nullptr && tcpSocket->state() == QAbstractSocket::ConnectedState){
            tcpSocket->disconnectFromHost();
        }
        tcpSocket->deleteLater();
    }
    if (tcpServer_->isListening()) {
        // 关闭网络监听
        tcpServer_->close();                     
    }
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

void NetworkManager::clearTcpSocketList()
{
    for(QTcpSocket *tcpSocket : tcpSocketList_) {
        if(tcpSocket != nullptr && tcpSocket->state() == QAbstractSocket::ConnectedState){
            tcpSocket->disconnectFromHost();
        }
        tcpSocket->deleteLater();
    }

    tcpSocketList_.clear();
}

void NetworkManager::loadLocalIP(QComboBox *comboBox)
{
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
    // 最后加入本地回环地址
    comboBox->addItem("127.0.0.1");
}

void NetworkManager::closeConnection()
{
    isOpen_ = false;
    curNetworkModel_ = None;

    if (tcpServer_->isListening()) {
        tcpServer_->close();
        clearTcpSocketList();
    }

    if (tcpClient_) {
        tcpClient_->abort();
    }
    udpSocket_->abort();
    emit sgn_btnStateChanged(false);
}

void NetworkManager::sendData(const QByteArray &content)
{
    if(curNetworkModel_ == TcpServer) {
        // 向所有客户端广播发送
        for(QTcpSocket *tcpSocket : tcpSocketList_) {
            if(tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState)
                tcpSocket->write(content);
        }
    }
    else if(curNetworkModel_ == TcpClient && tcpClient_->state() == QAbstractSocket::ConnectedState){
        tcpClient_->write(content);
    }
    else if(curNetworkModel_ == UDP){
        QHostAddress targetAddr(udpTargetIP_);
        udpSocket_->writeDatagram(content, targetAddr, udpTargetPort_);
    }
}

int NetworkManager::getClientCnt()
{
    return tcpSocketList_.size();
}

// TCP使用
void NetworkManager::do_btnOpenClose(CurNetworkModel networkModel, QString ip, quint16 port)
{
    // 当前是关闭状态, 开启
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
        }
    }
    // 当前是开启状态, 关闭
    else{
        // 关闭服务端, 清空 Socket 列表
        if (tcpServer_->isListening()){
            tcpServer_->close();
            clearTcpSocketList();
        }
        // 关闭客户端
        if (tcpClient_->state() == QAbstractSocket::ConnectedState ||
            tcpClient_->state() == QAbstractSocket::ConnectingState ||
            tcpClient_->state() == QAbstractSocket::HostLookupState) {
            tcpClient_->abort();
        }
        udpSocket_->abort();
        curNetworkModel_ = None;
    }
    isOpen_ = !isOpen_;
    emit sgn_btnStateChanged(isOpen_);
}

// 单独UDP使用
void NetworkManager::do_btnOpenClose(quint16 localport, QString targetIp, quint16 targetPort)
{
    if(!isOpen_){
        curNetworkModel_ = UDP;
        // UDP绑定时自动绑定本机所有网卡IP。需要绑定本地端口，和记录目标ip和端口
        udpTargetIP_ = targetIp;
        udpTargetPort_ = targetPort;
        if (!udpSocket_->bind(localport)){
            emit sgn_showMessage("UDP连接失败: " + udpSocket_->errorString());
            return;
        }
        sgn_labelShowState("绑定端口："+QString::number(udpSocket_->localPort()));
    }
    else{
        udpSocket_->abort();
        curNetworkModel_ = None;
    }
    isOpen_ = !isOpen_;
    emit sgn_btnStateChanged(isOpen_);
}

void NetworkManager::do_newConnection()
{
    // 创建新客户端 socket, 加入列表
    QTcpSocket *tcpSocket = tcpServer_->nextPendingConnection();
    tcpSocketList_.push_back(tcpSocket);

    emit sgn_stateChange(tcpSocket->state());

    connect(tcpSocket, &QTcpSocket::disconnected, this, [this, tcpSocket]() {
        qInfo() << "client disconnected: " + tcpSocket->peerAddress().toString();
        tcpSocketList_.removeOne(tcpSocket);

        // Socket数为0, 真正断开连接
        if(tcpSocketList_.size() == 0)
            emit sgn_stateChange(tcpSocket->state());
        else
            emit sgn_stateChange(QAbstractSocket::ConnectedState);

        tcpSocket->deleteLater();
    });

    connect(tcpSocket, &QTcpSocket::readyRead, this, [this, tcpSocket]() {
        sgn_readyRead(tcpSocket->readAll());
    });
}

void NetworkManager::do_socketReadyRead()
{
    while(udpSocket_->hasPendingDatagrams()){
        qint64 datagramSize = udpSocket_->pendingDatagramSize();

        QByteArray datagram;
        datagram.resize(static_cast<int>(datagramSize));

        QHostAddress peerAddr;
        quint16 peerPort = 0;

        udpSocket_->readDatagram(datagram.data(), datagram.size(), &peerAddr, &peerPort);

        sgn_readyRead(datagram);
    }
}
