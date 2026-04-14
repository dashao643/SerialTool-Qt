#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialsettingdialog.h"

#include <QPushButton>
#include <QFontDialog>
#include <QSerialPortInfo>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    btnInit();
    uiInit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::uiInit()
{
    // 展示页面不可编辑
    ui->plainTextEdit_Show->setEnabled(false);
    // 初始化可用端口
    foreach(QSerialPortInfo portInfo,QSerialPortInfo::availablePorts()){
        ui->cbBox_PortNum->addItem(portInfo.portName()+":"+portInfo.description());
    }
    /// 无端口，打开按钮不可用
    ui->btn_OpenClose->setEnabled(!QSerialPortInfo::availablePorts().isEmpty());
    // 初始化标准串口波特率
    foreach(qint32 BaudRate,QSerialPortInfo::standardBaudRates()){
        ui->cbBox_PortBuad->addItem(QString::number(BaudRate));
    }
    /// 默认选择115200，随后根据设置文件
    defaultBaudRateIndex = QSerialPortInfo::standardBaudRates().size() - 3;
    ui->cbBox_PortBuad->setCurrentIndex(defaultBaudRateIndex);
}

/**
 * @brief 按钮关联槽函数
 */
void MainWindow::btnInit()
{
    connect(ui->btn_PortRefresh,&QPushButton::clicked,this,&MainWindow::cbBoxPortNumRefresh);
    connect(ui->btn_OpenClose,&QPushButton::clicked,this,&MainWindow::btnOpenClose);
}

void MainWindow::cbBoxPortNumRefresh()
{
    ui->cbBox_PortNum->clear();
    foreach(QSerialPortInfo portInfo,QSerialPortInfo::availablePorts()){
        ui->cbBox_PortNum->addItem(portInfo.portName()+":"+portInfo.description());
    }
    labelInfoRefresh("端口刷新成功");
}

void MainWindow::btnOpenClose()
{

}

// QAction槽函数
void MainWindow::on_actPortSetting_triggered()
{
    SerialSettingDialog settingDialog(this);
    settingDialog.exec();
}

void MainWindow::on_actClear_triggered()
{
    ui->plainTextEdit_Show->clear();
}


void MainWindow::on_actFont_triggered()
{
    bool isOk = false;
    QFont curFont = ui->plainTextEdit_Show->font();
    QFont selFont = QFontDialog::getFont(&isOk, curFont, this);
    if(isOk){
        ui->plainTextEdit_Show->setFont(selFont);
        labelInfoRefresh("字体设置成功");
    }
}

void MainWindow::labelInfoRefresh(QString strInfo)
{
    ui->label_Info->setText(strInfo);
    QTimer::singleShot(500,[=](){
        ui->label_Info->clear();
    });
}

