#include "serialsettingdialog.h"
#include "ui_serialsettingdialog.h"

#include <QListView>

SerialSettingDialog::SerialSettingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SerialSettingDialog)
{
    ui->setupUi(this);
    this->setFixedSize(250,250);
    this->setWindowTitle("串口其它参数配置");

    m_dataBits = QSerialPort::Data8;                // 数据位
    m_stopBits = QSerialPort::OneStop;              // 停止位
    m_parity = QSerialPort::NoParity;               // 校验位
    m_flowControl = QSerialPort::NoFlowControl;     // 控制流

    ui->cbBox_DataBit->setView(new QListView());
    ui->cbBox_StopBit->setView(new QListView());
    ui->cbBox_Parity->setView(new QListView());
    
    comboBoxInit();

    ui->cbBox_DataBit->setCurrentIndex(ui->cbBox_DataBit->findData(QSerialPort::Data8));
    ui->cbBox_StopBit->setCurrentIndex(ui->cbBox_StopBit->findData(QSerialPort::OneStop));
    ui->cbBox_Parity->setCurrentIndex(ui->cbBox_Parity->findData(QSerialPort::NoParity));
}

SerialSettingDialog::~SerialSettingDialog()
{
    delete ui;
}

void SerialSettingDialog::comboBoxInit()
{
    // 文本显示文字, 用户数据存对应枚举值

    // ================= 数据位 =================
    ui->cbBox_DataBit->clear();
    ui->cbBox_DataBit->addItem("5", QSerialPort::Data5);
    ui->cbBox_DataBit->addItem("6", QSerialPort::Data6);
    ui->cbBox_DataBit->addItem("7", QSerialPort::Data7);
    ui->cbBox_DataBit->addItem("8", QSerialPort::Data8);

    // ================= 停止位 =================
    ui->cbBox_StopBit->clear();
    ui->cbBox_StopBit->addItem("1",   QSerialPort::OneStop);
    ui->cbBox_StopBit->addItem("1.5", QSerialPort::OneAndHalfStop);
    ui->cbBox_StopBit->addItem("2",   QSerialPort::TwoStop);

    // ================= 校验位 =================
    ui->cbBox_Parity->clear();
    ui->cbBox_Parity->addItem("None",  QSerialPort::NoParity);
    ui->cbBox_Parity->addItem("Odd",   QSerialPort::OddParity);
    ui->cbBox_Parity->addItem("Even",  QSerialPort::EvenParity);
}

// 点击了确定，更新串口配置
void SerialSettingDialog::on_buttonBox_accepted()
{
    // 直接取出 Qt 枚举
    m_dataBits = (QSerialPort::DataBits)ui->cbBox_DataBit->currentData().toInt();
    m_stopBits = (QSerialPort::StopBits)ui->cbBox_StopBit->currentData().toInt();
    m_parity   = (QSerialPort::Parity)ui->cbBox_Parity->currentData().toInt();
}

