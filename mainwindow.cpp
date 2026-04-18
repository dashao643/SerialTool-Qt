#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialsettingdialog.h"
#include "customdatasendwidget.h"

#include <QPushButton>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QDir>
#include <QSerialPortInfo>
#include <QSettings>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QDate>

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
    loadConfig();
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

    /// 默认串口配置
    m_serialConfig.dataBits = QSerialPort::Data8;
    m_serialConfig.stopBits = QSerialPort::OneStop;
    m_serialConfig.parity = QSerialPort::NoParity;
    m_serialConfig.flowControl = QSerialPort::NoFlowControl;

    // 接收超时
    m_rxTimer = new QTimer(this);
    m_rxTimer->setSingleShot(true);
    connect(m_rxTimer, &QTimer::timeout, this, &MainWindow::do_showReceivedData);

    // 定时发送
    m_sendTimer = new QTimer(this);
    connect(m_sendTimer, &QTimer::timeout, this, &MainWindow::do_btnComSendData);

    m_settings = new QSettings("serial.ini", QSettings::IniFormat, this);
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

    // 串口开关按钮 绿色
    ui->btn_OpenClose->setStyleSheet("background-color: #aaff7f;");
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
void MainWindow::loadConfig()
{
    /// 窗口大小
    // this->resize(m_settings->value("MainWindow/Width", DEFAULT_WINDOW_WIDTH).toInt(),
    //              m_settings->value("MainWindow/Height", DEFAULT_WINDOW_HEIGHT).toInt());

    /// 波特率,默认选择 115200
    m_baudRateIndex = qMax(0, m_standardBaudRates.size() - 3);
    ui->cbBox_PortBuad->setCurrentIndex(
        m_settings->value("ComConfig/BaudIndex", m_baudRateIndex).toInt());

    /// 发送模式
    ui->rdBtn_SendHex->setChecked(
        m_settings->value("SendMode/Hex", true).toBool());
    ui->rdBtn_SendASCII->setChecked(
        m_settings->value("SendMode/ASCII", false).toBool());

    /// 加换行
    ui->ckBox_Addn->setChecked(
        m_settings->value("Send/addn",true).toBool());

    /// 加校验
    ui->cbBox_DataCheck->setCurrentIndex(
        m_settings->value("Send/CheckMode", CheckDataIndex::NONE_CHECK).toInt());

    /// 定时发送时间
    ui->spBox_SendTime->setValue(
        m_settings->value("Send/TimeInterval", 1000).toInt());

    /// 显示模式
    ui->rdBtn_ShowHex->setChecked(
        m_settings->value("ShowMode/Hex", true).toBool());
    ui->rdBtn_ShowASCII->setChecked(
        m_settings->value("ShowMode/ASCII", false).toBool());

    /// 接收超时
    ui->spBox_Timeout->setValue(
        m_settings->value("Receive/Timeout", 20).toInt());

    /// 解析格式
    // ui->cbBox_Proces->setCurrentIndex(
    //     m_settings->value("Receive/dataProcess", SendProtocol::CUSTOM).toInt());

    /// 串口其他设置

    /// 保存文件路径
    // m_saveTextFilePath = m_settings->value("ToolBar/filePath", QDir::currentPath()).toString();

    /// 字体设置
    QFont font = m_settings->value("ShowWindow/font", QFont("Microsoft YaHei UI", 12)).value<QFont>();
    ui->plainTextEdit_Show->setFont(font);

    /// 背景颜色
    QColor bgColor = m_settings->value("ShowWindow/bgColor", QColor(Qt::white)).value<QColor>();
    QPalette palette = ui->plainTextEdit_Show->palette();
    palette.setColor(QPalette::Base, bgColor);
    ui->plainTextEdit_Show->setPalette(palette);

    /// 窗口可见
    if(!m_settings->value("ToolBar/dockVisible", true).toBool()){
        on_actDockVisible_triggered(false);
        ui->actDockVisible->setChecked(false);
    }

    /// dock栏页面
    ui->tabWidget->setCurrentIndex(SendProtocol::CUSTOM);
    //     m_settings->value("TabWidget/tabIndex", SendProtocol::CUSTOM).toInt());

    /// 自定义项和自定义数据
    loadCustomItem();

    labelInfoRefresh("配置加载成功");
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
        QMessageBox::information(this,"提示",
                                 "当前串口已断开");
            return;
        }
        m_comPort->setPort(curPortInfo);
        /// 先配置，打开端口
        comPortSetting();
        if(!m_comPort->open(QIODeviceBase::ReadWrite)){
            QMessageBox::information(this,"提示",
                                     "当前串口无法打开\n"
                                     "请检查串口是否被占用");
            return;
        }
        /// 按钮变红色
        ui->btn_OpenClose->setStyleSheet("background-color: #f98a52;");
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
        ui->btn_OpenClose->setStyleSheet("background-color: #aaff7f;");
        ui->btn_OpenClose->setText("打开串口");
        labelInfoRefresh(m_curPortName+"已关闭");
        ui->cbBox_PortNum->setDisabled(false);
    }
    m_isPortOpen = !m_isPortOpen;

    ui->actPortSetting->setDisabled(m_isPortOpen);
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
    ui->plainTextEdit_Show->appendHtml(
        QString("[%1]<span style='color:red'>收←◆%2</span>")
            .arg(strTime, strReceive)
    );
    // 清空缓冲区，等待下一包
    m_receiveBuffer.clear();
}

/**
 * @brief 主界面发送按钮
 */
void MainWindow::do_btnComSendData()
{
    QString content = ui->plainTextEdit_Send->toPlainText();
    SendModel sendModel = HEX;

    if (ui->rdBtn_SendASCII->isChecked()){
        sendModel = ASCII;
    }
    sendData(content,sendModel);
}

// QAction槽函数
void MainWindow::on_actPortSetting_triggered()
{
    SerialSettingDialog settingDialog(this);
    if(settingDialog.exec() == QDialog::Accepted){
        m_serialConfig.dataBits = settingDialog.m_dataBits;
        m_serialConfig.stopBits = settingDialog.m_stopBits;
        m_serialConfig.parity = settingDialog.m_parity;
        qDebug()<<m_serialConfig.parity;
        labelInfoRefresh("串口配置修改成功");
    }
}

void MainWindow::on_actClear_triggered()
{
    ui->plainTextEdit_Show->clear();
    labelInfoRefresh("清除成功");
}

void MainWindow::on_actSave_triggered()
{
    if(ui->plainTextEdit_Show->toPlainText().isEmpty()){
        labelInfoRefresh("无内容，保存失败");
        return;
    }
    QString defaultName = QDate::currentDate().toString("yyyy-MM-dd_");
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存日志文件",
        QDir::currentPath() + QString("/%1.txt").arg(defaultName),  // 默认路径 + 默认文件名
        "文本文件 (*.txt);;所有文件 (*.*)"      // 文件格式
        );

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::information(this,"提示","保存失败");
        return;
    }

    // 把显示区的所有内容写进去
    QTextStream out(&file);
    out << ui->plainTextEdit_Show->toPlainText();
    file.close();

    labelInfoRefresh("日志保存成功：" + fileName);
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

void MainWindow::on_actBackgroundSetting_triggered()
{
    QColor currentColor = ui->plainTextEdit_Show->palette().color(QPalette::Base);
    QColor bgColor = QColorDialog::getColor(currentColor, this, "选择背景色");

    if (!bgColor.isValid()) {
        return;
    }

    QPalette palette = ui->plainTextEdit_Show->palette();
    palette.setColor(QPalette::Base, bgColor);
    ui->plainTextEdit_Show->setPalette(palette);
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
    /// 按钮 + 输入框的 widget
    CustomDataSendWidget *w = new CustomDataSendWidget;

    ui->listWidget->addItem(addItem);
    ui->listWidget->setItemWidget(addItem, w);
    /// 必须设置高度
    addItem->setSizeHint(QSize(0,60));
    connect(w, &CustomDataSendWidget::sendDataRequest,this,
        [=](const QString &content, SendModel model){
        QString str = content;
        sendData(str,model);
    });
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
    // menuList->addAction(ui->actAddComment);
    // menuList->addSeparator();
    menuList->addAction(ui->actDel);
    /// 在鼠标光标位置显示快捷菜单
    menuList->exec(QCursor::pos());
    /// 菜单显示完后，删除对象
    delete menuList;
}

/**
 * @brief 根据配置文件加载自定义项，默认五个项
 */
void MainWindow::loadCustomItem()
{
    // 清空界面
    ui->listWidget->clear();

    // 读取配置里保存了多少项
    int count = m_settings->value("CustomItems/Count", 0).toInt();

    // 如果配置里没有, 默认创建 5 个空项
    if (count <= 0) {
        for (int i = 0; i < 5; i++) {
            on_actAdd_triggered();
        }
        return;
    }

    // 有配置, 按配置逐条恢复
    for (int i = 0; i < count; i++) {
        on_actAdd_triggered();

        // 获取刚添加的 item 和 widget
        QListWidgetItem* item = ui->listWidget->item(i);
        CustomDataSendWidget* w = (CustomDataSendWidget*)ui->listWidget->itemWidget(item);

        // 从配置读取数据
        QString remark = m_settings->value(QString("CustomItems/%1/Remark").arg(i)).toString();
        QString content = m_settings->value(QString("CustomItems/%1/Content").arg(i)).toString();
        bool isHex = m_settings->value(QString("CustomItems/%1/IsHex").arg(i)).toBool();

        // 填回控件
        w->setRemark(remark);
        w->setContent(content);
        if(isHex)
            w->setModel(SendModel::HEX);
        else
            w->setModel(SendModel::ASCII);
    }
}

void MainWindow::saveCustomItem()
{
    int count = ui->listWidget->count();
    m_settings->setValue("CustomItems/Count", count);

    for (int i = 0; i < count; i++) {
        QListWidgetItem* item = ui->listWidget->item(i);
        CustomDataSendWidget* w = (CustomDataSendWidget*)ui->listWidget->itemWidget(item);

        // 读取控件里的内容
        QString remark = w->getRemark();
        QString content = w->getContent();
        bool isHex = false;
        if(w->getModel() == SendModel::HEX)
            isHex = true;

        // 写入配置
        m_settings->setValue(QString("CustomItems/%1/Remark").arg(i), remark);
        m_settings->setValue(QString("CustomItems/%1/Content").arg(i), content);
        m_settings->setValue(QString("CustomItems/%1/IsHex").arg(i), isHex);
    }
}

void MainWindow::saveConfig()
{
    // 窗口大小
    // m_settings->setValue("MainWindow/Width", this->width());
    // m_settings->setValue("MainWindow/Height", this->height());

    // 波特率
    m_settings->setValue("ComConfig/BaudIndex", ui->cbBox_PortBuad->currentIndex());

    // 发送模式
    m_settings->setValue("SendMode/Hex", ui->rdBtn_SendHex->isChecked());
    m_settings->setValue("SendMode/ASCII", ui->rdBtn_SendASCII->isChecked());

    // 发送自动加换行
    m_settings->setValue("Send/addn", ui->ckBox_Addn->isChecked());

    // 校验模式
    m_settings->setValue("Send/CheckMode", ui->cbBox_DataCheck->currentIndex());

    // 定时发送时间
    m_settings->setValue("Send/TimeInterval", ui->spBox_SendTime->value());

    // 显示模式
    m_settings->setValue("ShowMode/Hex", ui->rdBtn_ShowHex->isChecked());
    m_settings->setValue("ShowMode/ASCII", ui->rdBtn_ShowASCII->isChecked());

    // 接收超时
    m_settings->setValue("Receive/Timeout", ui->spBox_Timeout->value());

    // 接收解析格式
    // m_settings->setValue("Receive/dataProcess", ui->cbBox_Proces->currentIndex());

    // 文件保存路径
    // m_settings->setValue("ToolBar/filePath", m_saveTextFilePath);

    // 字体
    m_settings->setValue("ShowWindow/font", ui->plainTextEdit_Show->font());

    // 背景色
    m_settings->setValue("ShowWindow/bgColor",
                         ui->plainTextEdit_Show->palette().color(QPalette::Base));

    // 停靠窗口可见性
    m_settings->setValue("ToolBar/dockVisible", ui->actDockVisible->isChecked());

    // tab 页面索引
    // m_settings->setValue("TabWidget/tabIndex", ui->tabWidget->currentIndex());

    // 自定义项
    saveCustomItem();
}

/// 设置串口配置
void MainWindow::comPortSetting()
{
    m_comPort->setBaudRate(m_standardBaudRates.at(m_baudRateIndex));
    m_comPort->setDataBits(m_serialConfig.dataBits);
    m_comPort->setStopBits(m_serialConfig.stopBits);
    m_comPort->setParity(m_serialConfig.parity);
    m_comPort->setFlowControl(m_serialConfig.flowControl);
}

void MainWindow::sendData(QString &content, SendModel model)
{
    if(!m_comPort->isOpen()){
        labelInfoRefresh("串口未打开，发送失败");
        return;
    }

    QByteArray sendBuf;

    if (model == ASCII){
        if(ui->ckBox_Addn->isChecked())
            content.append("\r\n");
        /// UTF8 或 Local8Bit
        sendBuf = content.toLocal8Bit();
    }
    else if(model == HEX){
        /// 去掉所有空格
        content.remove(" ");
        /// 过滤掉 0-9、A-F、a-f 以外的所有非法字符
        content.remove(QRegularExpression("[^0-9a-fA-F]"));
        sendBuf = QByteArray::fromHex(content.toUtf8());
        /// 加校验
        appendCheckData(sendBuf);
    }
    if (sendBuf.isEmpty()) {
        labelInfoRefresh("发送内容不能为空");
        return;
    }

    m_comPort->write(sendBuf);
    /// 发送内容展示
    showSendData(sendBuf);
    labelInfoRefresh(QString("发送成功 %1 字节").arg(sendBuf.size()));
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
    // 蓝色发送
    ui->plainTextEdit_Show->appendHtml(
        QString("[%1]<span style='color:blue'>发→◇%2</span>")
            .arg(strTime, strSend)
    );
}

void MainWindow::labelInfoRefresh(const QString strInfo)
{
    ui->label_Info->setText(strInfo);
    m_infoTimer->start(1000);
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveConfig();
    labelInfoRefresh("配置保存成功");
    event->accept();
}

/***************** 发送模块 *****************/

/// Modbus
void MainWindow::on_btn_ModbusReadAdd_clicked()
{

}

void MainWindow::on_lineEdit_SlaveAddress_textChanged(const QString &arg1)
{
    m_modbus->setSlaveAddress(arg1);
}

void MainWindow::on_btn_ModbusRedOn_clicked()
{
    QString cmd = m_modbus->LED_RED_ON();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_ModbusRedOff_clicked()
{
    QString cmd = m_modbus->LED_RED_OFF();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_ModbusRedToggle_clicked()
{
    QString cmd = m_modbus->LED_RED_TOGGLE();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_ModbusGreenOn_clicked()
{
    QString cmd = m_modbus->LED_GREEN_ON();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_ModbusGreenOff_clicked()
{
    QString cmd = m_modbus->LED_GREEN_OFF();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_ModbusGreenToggle_clicked()
{
    QString cmd = m_modbus->LED_GREEN_TOGGLE();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_ModbusBlueOn_clicked()
{
    QString cmd = m_modbus->LED_BLUE_ON();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_ModbusBlueOff_clicked()
{
    QString cmd = m_modbus->LED_BLUE_OFF();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_ModbusBlueToggle_clicked()
{
    QString cmd = m_modbus->LED_BLUE_TOGGLE();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_DHT11ReadTemp_clicked()
{
    QString cmd = m_modbus->DHT11_READ_TEMP();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_DHT11ReadHumi_clicked()
{
    QString cmd = m_modbus->DHT11_READ_HUMI();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_DHT11ReadTH_clicked()
{
    QString cmd = m_modbus->DHT11_READ_TH();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_RTCSetTime_clicked()
{
    QString cmd = m_modbus->RTC_SET_TIME();
    sendData(cmd, SendModel::HEX);
}

void MainWindow::on_btn_RTCGetTime_clicked()
{
    QString cmd = m_modbus->RTC_GET_TIME();
    sendData(cmd, SendModel::HEX);
}
