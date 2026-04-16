#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialsettingdialog.h"
#include "customdatasendwidget.h"

#include <QPushButton>
#include <QFontDialog>
#include <QSerialPortInfo>
#include <QSettings>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<this->width()<<" "<<this->height();
    this->setWindowTitle(QString("串口工具-v%1").arg(APP_VERSION));

    dataInit();
    slotsInit();
    uiInit();
    iniFileInit();
}

MainWindow::~MainWindow()
{
    if (m_comPort->isOpen())
        m_comPort->close();
    delete ui;
}

void MainWindow::dataInit()
{
    m_comPort = new QSerialPort(this);
    m_standardBaudRates = QSerialPortInfo::standardBaudRates();

    // 状态栏提示定时器
    m_infoTimer = new QTimer(this);
    m_infoTimer->setSingleShot(true);
    connect(m_infoTimer,&QTimer::timeout,ui->label_Info,&QLabel::clear);

    // 默认波特率索引
    // if(配置文件中内容存在){
    //     defaultBaudRateIndex = 配置文件中内容
    // }
    /// 无配置文件，默认选择115200
    // else{
        m_defaultBaudRateIndex = qMax(0, m_standardBaudRates.size() - 3);
    // }

    /// 默认串口配置
    m_comPort->setDataBits(QSerialPort::Data8);
    m_comPort->setStopBits(QSerialPort::OneStop);
    m_comPort->setParity(QSerialPort::NoParity);
    m_comPort->setFlowControl(QSerialPort::NoFlowControl);

    // 接收超时
    m_rxTimer = new QTimer(this);
    m_rxTimer->setSingleShot(true);
    connect(m_rxTimer, &QTimer::timeout, this, &MainWindow::do_showReceivedData);

    // 定时发送
    m_sendTimer = new QTimer(this);
    connect(m_sendTimer, &QTimer::timeout, this, &MainWindow::do_btnComSendData);

    showWindowFont = ui->plainTextEdit_Show->font();
    showWindowFont.setPointSize(12);
    ui->plainTextEdit_Show->setFont(showWindowFont);
}

void MainWindow::uiInit()
{
    // 展示页面不可编辑
    ui->plainTextEdit_Show->setReadOnly(true);
    // 初始先刷新端口
    do_cbBoxPortNumRefresh();

    // 初始化标准串口波特率
    foreach(qint32 BaudRate,QSerialPortInfo::standardBaudRates()){
        ui->cbBox_PortBuad->addItem(QString::number(BaudRate));
    }

    ui->cbBox_PortBuad->setCurrentIndex(m_defaultBaudRateIndex);
    ui->tabWidget->setCurrentIndex(sendModel::CUSTOM);
}

/**
 * @brief 槽函数初始化
 */
void MainWindow::slotsInit()
{
    // 按键槽函数
    connect(ui->btn_PortRefresh,&QPushButton::clicked,this,&MainWindow::do_cbBoxPortNumRefresh);
    connect(ui->btn_OpenClose,&QPushButton::clicked,this,&MainWindow::do_btnOpenClose);
    connect(ui->btn_Send,&QPushButton::clicked,this,&MainWindow::do_btnComSendData);
    // 串口设备槽函数
    connect(m_comPort,&QSerialPort::readyRead,this,&MainWindow::do_comReadyRead);

    /// Hex 发送时禁用自动换行
    connect(ui->rdBtn_SendHex, &QRadioButton::clicked, this, [=](bool checked){
        ui->ckBox_Addn->setEnabled(!checked);
    });
    /// ASCII 发送时启用自动换行
    connect(ui->rdBtn_SendASCII, &QRadioButton::clicked, this, [=](bool checked){
        ui->ckBox_Addn->setEnabled(checked);
    });
}

/**
 * @brief 根据配置文件，初始化所有设置
 */
void MainWindow::iniFileInit()
{
    // iniFile.setFileName("serial.ini");
    // if(iniFile.open())
    // QSettings settingFile("",QSettings::IniFormat,this);

    ui->ckBox_Addn->setDisabled(ui->rdBtn_SendHex->isChecked());
}

// 刷新串口列表
void MainWindow::do_cbBoxPortNumRefresh()
{
    ui->cbBox_PortNum->clear();
    m_curAvailablePorts = QSerialPortInfo::availablePorts();

    foreach(QSerialPortInfo portInfo,m_curAvailablePorts){
        ui->cbBox_PortNum->addItem(portInfo.portName()+":"+portInfo.description());
    }
    /// 无端口，打开按钮不可用
    ui->btn_OpenClose->setDisabled(m_curAvailablePorts.isEmpty());
    /// 如果原来端口已打开，自动关闭
    if(m_isPortOpen)
        do_btnOpenClose();

    labelInfoRefresh("端口刷新完成");
}

void MainWindow::do_btnOpenClose()
{
    // 当前端口未打开
    if(!m_isPortOpen){
        /// 当前选择的端口
        int idx = ui->cbBox_PortNum->currentIndex();
        if (idx < 0 || idx >= m_curAvailablePorts.size()) {
            labelInfoRefresh("未选择有效串口");
            return;
        }
        QSerialPortInfo curPortInfo = m_curAvailablePorts.at(idx);
        m_curPortName = curPortInfo.portName();

        /// 创建临时变量，再检测下可用端口
        bool exist = false;
        for (const auto& p : QSerialPortInfo::availablePorts()) {
            if (p.portName() == m_curPortName) {
                exist = true;
                break;
            }
        }
        if(!exist){
            labelInfoRefresh(m_curPortName+"串口已断开，请刷新");
            return;
        }
        /// 当前存在，，先配置，打开端口
        m_comPort->setPort(curPortInfo);
        comPortSetting();
        if(!m_comPort->open(QIODeviceBase::ReadWrite)){
            QMessageBox::information(this,"提示",
                                     "当前串口号无法打开！\n"
                                     "请检查串口是否被占用或已断开");
            return;
        }
        /// 按钮变红色
        ui->btn_OpenClose->setText("关闭串口");
        labelInfoRefresh(m_curPortName+"已打开");
        /// 打开此端口后，不能选择其他端口
        ui->cbBox_PortNum->setDisabled(true);
    }
    else{
        /// 按钮绿色
        m_comPort->close();
        // 关闭清空缓冲区
        m_receiveBuffer.clear();
        ui->btn_OpenClose->setText("打开串口");
        labelInfoRefresh(m_curPortName+"已关闭");
        ui->cbBox_PortNum->setDisabled(false);
    }
    m_isPortOpen = !m_isPortOpen;
}

/**
 * @brief 缓冲区接收到数据
 */
void MainWindow::do_comReadyRead()
{
    /// 先把数据读到缓冲区 append不会自动加换行，plainTextEdit显示会自动加
    m_receiveBuffer.append(m_comPort->readAll());

    // 启动超时时间定时器
    m_rxTimer->start(ui->spBox_Timeout->value());
}

void MainWindow::do_showReceivedData()
{
    if (m_receiveBuffer.isEmpty()) return;

    QString strTime = QTime::currentTime().toString("HH:mm:ss.zzz");

    QString strReceive;
    if(ui->rdBtn_ShowASCII->isChecked()){
        strReceive = QString::fromUtf8(m_receiveBuffer);
    }
    else if(ui->rdBtn_ShowHex->isChecked()){
        strReceive = m_receiveBuffer.toHex(' ').toUpper();;
    }
    ui->plainTextEdit_Show->appendPlainText(QString("[%1]收←◆%2").arg(strTime, strReceive));
    // 清空缓冲区，等待下一包
    m_receiveBuffer.clear();
}

void MainWindow::do_btnComSendData()
{
    if(!m_comPort->isOpen()){
        labelInfoRefresh("串口未打开，发送失败");
        return;
    }
    QByteArray sendBuf;
    QString text = ui->plainTextEdit_Send->toPlainText();

    // ASCII发送
    if (ui->rdBtn_SendASCII->isChecked()){
        if(ui->ckBox_Addn->isChecked())
            text.append("\r\n");
        /// UTF8 或 Local8Bit
        sendBuf = text.toLocal8Bit();
    }
    // Hex发送
    else if(ui->rdBtn_SendHex->isChecked()){
        /// 去掉所有空格
        text.remove(" ");
        /// 过滤掉 0-9、A-F、a-f 以外的所有非法字符
        text.remove(QRegularExpression("[^0-9a-fA-F]"));
        /// 转成 QByteArray
        sendBuf = QByteArray::fromHex(text.toUtf8());
        appendCheckData(sendBuf);
    }
    if (sendBuf.isEmpty()) {
        labelInfoRefresh("发送内容不能为空");
        return;
    }

    m_comPort->write(sendBuf);
    // 发送内容展示
    showSendData(sendBuf);
    labelInfoRefresh(QString("发送成功 %1 字节").arg(sendBuf.size()));
}

// QAction槽函数
void MainWindow::on_actPortSetting_triggered()
{
    SerialSettingDialog settingDialog(this);
    if(settingDialog.exec() == QDialog::Accepted){
        qDebug() << "数据位：" << settingDialog.m_dataBits;
        qDebug() << "停止位：" << settingDialog.m_stopBits;
        qDebug() << "校验位：" << settingDialog.m_parity;

        m_comPort->setDataBits(settingDialog.m_dataBits);
        m_comPort->setStopBits(settingDialog.m_stopBits);
        m_comPort->setParity(settingDialog.m_parity);

        labelInfoRefresh("串口配置修改成功");
    }
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
    else{
        labelInfoRefresh("字体设置失败");
    }
}

void MainWindow::on_actDockFloat_triggered(bool checked)
{
    ui->dockWidget->setFloating(checked);
}

void MainWindow::on_actDockVisible_triggered(bool checked)
{
    ui->dockWidget->setVisible(checked);
}

void MainWindow::on_actAdd_triggered()
{
    QListWidgetItem *addItem = new QListWidgetItem;
    ui->listWidget->addItem(addItem);

    // 自己做一个带按钮 + 输入框的 widget
    CustomDataSendWidget *w = new CustomDataSendWidget;
    // 把这个带内容的 widget 放进去
    ui->listWidget->setItemWidget(addItem, w);

    // 必须设置高度！
    addItem->setSizeHint(QSize(0,60));
}

void MainWindow::on_actDel_triggered()
{
    int row = ui->listWidget->currentRow();
    if(row == -1){
        labelInfoRefresh("未选中，删除项失败");
        return;
    }
    QListWidgetItem* item = ui->listWidget->takeItem(row);
    delete item;
    labelInfoRefresh("删除项成功");
}

void MainWindow::on_actAddComment_triggered()
{

}

void MainWindow::on_cbBox_PortBuad_currentIndexChanged(int index)
{
    m_comPort->setBaudRate(m_standardBaudRates.at(index));
    labelInfoRefresh("波特率已修改:" + QString::number(m_standardBaudRates.at(index)));
}

void MainWindow::on_ckBox_SendByTime_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1 == Qt::Checked) {
        if(!m_isPortOpen){
            QMessageBox::information(this,"提示", "当前串口未打开");
            /// 取消勾选
            ui->ckBox_SendByTime->setChecked(false);
            return;
        }
        int ms = ui->spBox_SendTime->value();
        m_sendTimer->start(ms);
        labelInfoRefresh("定时发送已启动：" + QString::number(ms) + "ms");
    }
    else {
        m_sendTimer->stop();
        labelInfoRefresh("已停止定时发送");
    }
}

// listWidget右键菜单槽函数
void MainWindow::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu *menuList = new QMenu(this);
    menuList->addAction(ui->actAddComment);
    // menuList->addSeparator();
    // menuList->addAction(ui->actDel);
    /// 在鼠标光标位置显示快捷菜单
    menuList->exec(QCursor::pos());
    /// 菜单显示完后，删除对象
    delete menuList;
}

/// 设置串口配置
void MainWindow::comPortSetting()
{
    m_comPort->setFlowControl(QSerialPort::NoFlowControl);
}

/**
 * @brief 发送数据结尾自动加校验
 */
void MainWindow::appendCheckData(QByteArray &sendBuf)
{
    int curIndex = ui->cbBox_DataCheck->currentIndex();
    if(curIndex == CheckDataIndex::NONE_CHECK)
        return;
    else if(curIndex == CheckDataIndex::MODBUS_CRC16){
        appendModbusCRC16(sendBuf);
    }
    else if(curIndex == CheckDataIndex::ADD8){
        appendAdd8(sendBuf);
    }
}

void MainWindow::showSendData(const QByteArray &sendBuf)
{
    QString strTime = QTime::currentTime().toString("HH:mm:ss.zzz");
    QString strSend;
    if(ui->rdBtn_ShowASCII->isChecked()){
        strSend = QString::fromUtf8(sendBuf);
    }
    else if(ui->rdBtn_ShowHex->isChecked()){
        strSend = sendBuf.toHex(' ').toUpper();;
    }
    ui->plainTextEdit_Show->appendPlainText(QString("[%1]发→◇%2").arg(strTime, strSend));
}

void MainWindow::labelInfoRefresh(const QString strInfo)
{
    ui->label_Info->setText(strInfo);
    m_infoTimer->start(1000);
}

void MainWindow::appendModbusCRC16(QByteArray &data)
{
    // Modbus CRC 初始值
    quint16 crc = 0xFFFF;

    for (char ch : data) {
        crc ^= static_cast<quint8>(ch);

        for (int i = 0; i < 8; i++) {
            if (crc & 1) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    // Modbus CRC 低字节在前，高字节在后
    data.append(static_cast<char>(crc & 0xFF));             // 低字节
    data.append(static_cast<char>((crc >> 8) & 0xFF));      // 高字节
}

void MainWindow::appendAdd8(QByteArray &data)
{
    quint8 sum = 0;
    for(auto val : data){
        sum += val;
    }
    // QByteArray需要 signed char 类型
    data.append(static_cast<char>(sum));
}



