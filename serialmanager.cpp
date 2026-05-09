#include "serialmanager.h"

#include <QSerialPortInfo>
#include <QMessageBox>

SerialManager::SerialManager(QObject *parent)
    : QObject{parent}
{
    dataInit();
    slotsInit();
}

void SerialManager::dataInit()
{
    serialPort_ = new QSerialPort(this);
    serialConfig_.dataBits    = QSerialPort::Data8;
    serialConfig_.stopBits    = QSerialPort::OneStop;
    serialConfig_.parity      = QSerialPort::NoParity;
    serialConfig_.flowControl = QSerialPort::NoFlowControl;
}

void SerialManager::slotsInit()
{
    connect(this,&SerialManager::serialStateChanged,[=](bool isOpen){
        isPortOpen_ = isOpen;
        if(isOpen){
            // serialPort_->open()
            // serialPort_->errorString();
        }
        else{
            serialPort_->close();
        }
    });
}

void SerialManager::baudRateInit(QComboBox *comboBox)
{
    foreach(qint32 BaudRate,QSerialPortInfo::standardBaudRates()){
        comboBox->addItem(QString::number(BaudRate));
    }
}

void SerialManager::sendData(const QByteArray &content)
{
    if(!isPortOpen_){
        emit showMessage("端口未开启");
        return;
    }
    serialPort_->write(content);
}

void SerialManager::do_btnPortRefresh(QComboBox *comboBox)
{
    comboBox->clear();

    foreach(QSerialPortInfo portInfo, QSerialPortInfo::availablePorts()){
        comboBox->addItem(portInfo.portName()+":"+portInfo.description());
    }
    emit serialStateChanged(false);
    emit availablePort(QSerialPortInfo::availablePorts().isEmpty());
    emit labelShowInfo("端口刷新完成");
}

void SerialManager::do_btnOpenClose(QComboBox *comboBoxPort, const QComboBox *comboBoxBuadRate)
{
    if(!isPortOpen_){
        int curSize = comboBoxPort->count();
        int curIndex = comboBoxPort->currentIndex();
        do_btnPortRefresh(comboBoxPort);
        if(curSize != comboBoxPort->count()){
            emit showMessage("串口已更新，请重新选择");
            return;
        }
        comboBoxPort->setCurrentIndex(curIndex);
        QSerialPortInfo curPortInfo = QSerialPortInfo::availablePorts().at(curIndex);
        serialPort_->setPort(curPortInfo);
        /// 先配置，再打开端口
        comPortSetting(comboBoxBuadRate);
        if(!serialPort_->open(QIODeviceBase::ReadWrite)){
            emit showMessage("当前串口无法打开\n"
                             "请检查串口是否被占用");
            return;
        }
        emit labelShowInfo(curPortInfo.portName() + "已开启");
    }
    else{
        serialPort_->close();
        emit labelShowInfo(serialPort_->portName() + "已关闭");
    }

    isPortOpen_ = !isPortOpen_;
    emit serialStateChanged(isPortOpen_);
}

void SerialManager::setBuadRate(QComboBox *comboBox, int index)
{
    int buadRate = comboBox->itemText(index).toInt();
    serialPort_->setBaudRate(buadRate);
    emit labelShowInfo("波特率已修改:" + QString::number(buadRate));
}

void SerialManager::comPortSetting(const QComboBox *comboBoxBuadRate)
{
    serialPort_->setBaudRate(comboBoxBuadRate->currentText().toInt());
    serialPort_->setDataBits(serialConfig_.dataBits);
    serialPort_->setStopBits(serialConfig_.stopBits);
    serialPort_->setParity(serialConfig_.parity);
    serialPort_->setFlowControl(serialConfig_.flowControl);
}
