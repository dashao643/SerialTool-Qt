#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customitem.h"
#include "serialsettingdialog.h"
#include "sendfiledialog.h"

#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTime>
#include <QScrollBar>
#include <QDir>
#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QThread>
#include <QProgressDialog>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<this->width()<<" "<<this->height();
    this->setWindowTitle(QString("串口工具-v%1").arg(APP_VERSION));

    dataInit();
    uiInit();
    slotsInit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dataInit()
{
    appConfig_ = new AppConfig(this);
    serialManager_ = new SerialManager(this);

    infoTimer_ = new QTimer(this);
    infoTimer_->setSingleShot(true);

    rxTimer_ = new QTimer(this);
    rxTimer_->setSingleShot(true);

    sendTimer_ = new QTimer(this);

    fileSavePath_ = QDir::currentPath();
}

void MainWindow::uiInit()
{
    // 展示页面不可编辑
    ui->plainTextEdit_Show->setReadOnly(true);
    // 初始先刷新端口
    serialManager_->do_btnPortRefresh(ui->cbBox_PortNum);
    // 初始化标准串口波特率
    serialManager_->baudRateInit(ui->cbBox_PortBuad);
    ui->tabWidget->clear();
    loadPageConfig();
    ui->tabWidget->setCurrentIndex(0);
    ui->stackedWidget->setCurrentIndex(0);
    ui->cbBox_Model->setCurrentIndex(0);
    do_UiUpdate(false);
    // 加载配置
    Config_t config = appConfig_->loadConfig();
    sendFile_ = config.sendFile;
    // this->resize(config.windowSize);
    ui->cbBox_PortBuad->setCurrentIndex(config.baudRateIndex);
    ui->cbBox_DataCheck->setCurrentIndex(config.check);
    fileSavePath_ = config.filePath;
    ui->plainTextEdit_Show->setFont(config.font);
    ui->dockWidget->setVisible(config.isDockVisible);
}

void MainWindow::slotsInit()
{
    // 标签栏提示
    connect(infoTimer_,&QTimer::timeout,ui->label_Info,&QLabel::clear);
    connect(serialManager_,&SerialManager::labelShowInfo,this,&MainWindow::labelInfoRefresh);
    // 消息框提示
    connect(serialManager_,&SerialManager::showMessage,[=](const QString &string){
        QMessageBox::information(this, "提示", string);
    });
    // ui更新
    connect(serialManager_,&SerialManager::serialStateChanged,this,&MainWindow::do_UiUpdate);
    // 串口开关按键状态
    connect(serialManager_,&SerialManager::availablePort,[=](bool isEmpty){
        ui->btn_OpenClose->setDisabled(isEmpty);
    });
    // 刷新端口
    connect(ui->btn_PortRefresh,&QPushButton::clicked,serialManager_,[=](){
        serialManager_->do_btnPortRefresh(ui->cbBox_PortNum);
    });
    // 串口配置
    connect(ui->btn_OpenClose,&QPushButton::clicked,this,[=](){
        serialManager_->do_btnOpenClose(ui->cbBox_PortNum, ui->cbBox_PortBuad);
    });
    // 选择模式
    connect(ui->cbBox_Model,&QComboBox::currentIndexChanged,this,[=](int index){
        ui->stackedWidget->setCurrentIndex(index);
    });
    // 串口有数据 -> 串口读数据
    connect(serialManager_->serialPort_,&QSerialPort::readyRead,this,&MainWindow::do_serialReadyRead);
    // 累计读出数据，经过超时时间，显示数据
    connect(rxTimer_, &QTimer::timeout, this, &MainWindow::do_showReceivedData);
    // textEdit 选中 -> 计算选中个数
    connect(ui->plainTextEdit_Show,&QPlainTextEdit::selectionChanged,
            this,&MainWindow::do_calSelCharCnt);
    // 定时发送
    connect(sendTimer_, &QTimer::timeout, this, &MainWindow::on_btn_Send_clicked);
    connect(ui->ckBox_SendByTime,&QCheckBox::clicked,sendTimer_,&QTimer::stop);
    // 工具栏按钮
    connect(ui->actClear,&QAction::triggered,ui->plainTextEdit_Show,&QPlainTextEdit::clear);
    connect(ui->actWindowReset,&QAction::triggered,[=](){
        this->resize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    });
    connect(ui->actDockFloat,&QAction::triggered,ui->dockWidget,&QDockWidget::setFloating);
    connect(ui->actDockVisible,&QAction::triggered,ui->dockWidget,&QDockWidget::setVisible);
    connect(ui->actAddItem,&QAction::triggered,this,[=](){
        do_addItemToList();
    });
    connect(ui->actSendFile,&QAction::triggered,this,[=](){
        sendFile_.model = ui->cbBox_Model->currentIndex();
        SendFileDialog dialog(sendFile_, this);
        connect(&dialog,&SendFileDialog::download,this,&MainWindow::do_fileDownload);
        dialog.exec();
        isFileDownload = false;
        sendFile_ = dialog.getConfig();
    });
    // 波特率修改
    connect(ui->cbBox_PortBuad,&QComboBox::currentIndexChanged,serialManager_,[=](int index){
        serialManager_->setBuadRate(ui->cbBox_PortBuad, index);
    });
}

void MainWindow::loadPageConfig()
{
    QList<TabPageConfig> tabPageList = appConfig_->loadTabPage();
    for (TabPageConfig& page : tabPageList) {
        TabPage *tabPage = new TabPage();
        ui->tabWidget->addTab(tabPage, page.name);
        ui->tabWidget->setCurrentWidget(tabPage);
        connect(tabPage, &TabPage::insertItem, this, [=](int index){
            do_addItemToList(index, true);
        });
        QListWidget *list = tabPage->getListWidget();
        for(int i = 0; i < page.items.count(); i++){
            do_addItemToList();
            // 获取刚刚创建的项
            auto* item = list->item(i);
            auto* customItem = qobject_cast<CustomItem*>(list->itemWidget(item));
            auto& cfg = page.items[i];
            customItem->setRemark(cfg.remark);
            customItem->setContent(cfg.content);
            customItem->setModel(cfg.model);
        }
    }
}

// 信号槽绑定的参数，要么传值，要么传 const + 引用
void MainWindow::sendData(QString content, SendModel model)
{
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
        if(!isFileDownload)
            appendCheckData(sendBuf);
    }
    if (sendBuf.isEmpty()) {
        labelInfoRefresh("发送内容不能为空");
        return;
    }

    // 串口发送
    if(ui->cbBox_Model->currentIndex() == 0){
        serialManager_->sendData(sendBuf);
    }
    // 网络发送
    else if(ui->cbBox_Model->currentIndex() == 1){

    }
    if(!serialManager_->isPortOpen_)
        return;
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
    infoTimer_->start(LABEL_INFO_SHOW_MS);
}

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
            }
            else {
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

bool MainWindow::waitAck(const QString &ackStr, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray targetAck = QByteArray::fromHex(ackStr.toUtf8());

    while (timer.elapsed() < timeoutMs) {
        // 让程序继续处理事件（串口、UI、定时器）
        QCoreApplication::processEvents();

        // 检查是否收到正确 ACK
        if (fileReceiveBuffer_.contains(targetAck)) {
            fileReceiveBuffer_.clear();
            return true;
        }

        QThread::msleep(5);
    }

    return false;
}

// 更新所有有关串口开关状态的显示
void MainWindow::do_UiUpdate(bool isSerialOpen)
{
    if(isSerialOpen){
        ui->btn_OpenClose->setText("关闭串口");
        ui->btn_OpenClose->setStyleSheet(R"(
            QPushButton{
                background-color: #f98a52;
                border-radius: 4px;
                border: none;
                padding:6px;
            }
            QPushButton:hover{
                background-color: #fFAa72;
            }
            QPushButton:pressed{
                background-color: #D96a32;
            }
        )");
    }
    else{
        ui->btn_OpenClose->setText("打开串口");
        ui->btn_OpenClose->setStyleSheet(R"(
        QPushButton {
            background-color: #aaff7f;
            border-radius: 4px;
            border: none;
            padding:6px;
        }
        QPushButton:hover{
            background-color: #caff9f;
        }
        QPushButton:pressed{
            background-color: #8adf5f;
        }
        )");
    }
    ui->cbBox_PortNum->setDisabled(isSerialOpen);
    ui->actPortSetting->setDisabled(isSerialOpen);
    ui->btn_Send->setEnabled(isSerialOpen);
}

void MainWindow::do_serialReadyRead()
{
    /// 先把数据读到缓冲区 append不会自动加换行，plainTextEdit显示会自动加
    QByteArray tempData = serialManager_->serialPort_->readAll();

    receiveBuffer_.append(tempData);
    rxTimer_->start(ui->spBox_Timeout->value());

    if (isFileDownload) {
        fileReceiveBuffer_.append(tempData);
    }
}

void MainWindow::do_showReceivedData()
{
    if (receiveBuffer_.isEmpty()) return;

    QString strTime = QTime::currentTime().toString("HH:mm:ss.zzz");

    QString strReceive;
    if(ui->rdBtn_ShowASCII->isChecked()){
        strReceive = QString::fromUtf8(receiveBuffer_);
    }
    else if(ui->rdBtn_ShowHex->isChecked()){
        strReceive = receiveBuffer_.toHex(' ').toUpper();;
    }
    ui->plainTextEdit_Show->appendHtml(
        QString("[%1]<span style='color:red'>收←◆%2</span>")
            .arg(strTime, strReceive)
        );
    labelInfoRefresh(QString("接收了%1字节").arg(receiveBuffer_.size()));
    // 清空缓冲区，等待下一包
    receiveBuffer_.clear();
    // 显示区显示最后
    QScrollBar *scrollBar = ui->plainTextEdit_Show->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::do_calSelCharCnt()
{
    // 获取当前鼠标选中的文字
    QString selectedText = ui->plainTextEdit_Show->textCursor().selectedText();
    if(selectedText.isEmpty())
        return;
    // 按空格/换行分割，统计有效数据个数
    QStringList list = selectedText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    labelInfoRefresh(QString("当前选中个数：%1").arg(list.size()));
}

// 根据当前选择的模式发送数据
void MainWindow::on_btn_Send_clicked()
{
    QString content = ui->plainTextEdit_Send->toPlainText();
    SendModel sendModel = HEX;

    if (ui->rdBtn_SendASCII->isChecked()){
        sendModel = ASCII;
    }
    sendData(content,sendModel);

    if(ui->ckBox_SendByTime->isChecked()){
        int ms = ui->spBox_SendTime->value();
        sendTimer_->start(ms);
    }
}

// 将选中的部分的十六进制数，转化成 ascii 码显示
void MainWindow::on_rdBtn_ShowASCII_clicked()
{
    QTextCursor cursor = ui->plainTextEdit_Show->textCursor();
    QString selected = cursor.selectedText().trimmed();

    if (selected.isEmpty()) return;

    selected.remove(" ");
    selected.remove(QRegularExpression("[^0-9a-fA-F]"));

    if (selected.isEmpty()) return;

    QByteArray hexBytes = selected.toUtf8();
    QString ascii = QString::fromLocal8Bit(hexBytes);

    cursor.insertText(ascii);

    labelInfoRefresh("Hex 转换 ASCII 成功");
}

void MainWindow::on_actPortSetting_triggered()
{
    SerialSettingDialog settingDialog(this);
    if(settingDialog.exec() == QDialog::Accepted){
        serialManager_->serialConfig_.dataBits = settingDialog.m_dataBits;
        serialManager_->serialConfig_.stopBits = settingDialog.m_stopBits;
        serialManager_->serialConfig_.parity = settingDialog.m_parity;
        labelInfoRefresh("串口配置修改成功");
    }
}

void MainWindow::on_actSave_triggered()
{
    if(ui->plainTextEdit_Show->toPlainText().isEmpty()){
        QMessageBox::information(this,"提示","当前无内容");
        return;
    }
    QString defaultName = QDate::currentDate().toString("yyyy-MM-dd_");
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存日志文件",
        fileSavePath_ + QString("/%1.txt").arg(defaultName),  // 默认路径 + 默认文件名
        "文本文件 (*.txt);;所有文件 (*.*)"      // 文件格式
        );
    if(fileName.isEmpty())  return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::information(this,"提示","保存失败");
        return;
    }
    // 把显示区的所有内容写进去
    QTextStream out(&file);
    out << ui->plainTextEdit_Show->toPlainText();
    file.close();

    fileSavePath_ = QFileInfo(fileName).absolutePath();
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

void MainWindow::on_actAddTab_triggered()
{
    bool isOk = false;
    QString defaultName = "tab" + QString::number(ui->tabWidget->count() + 1);
    QString input = QInputDialog::getText(this,"输入","页面名称:",
                                          QLineEdit::Normal, defaultName,&isOk);
    if(!isOk || input.isEmpty()) return;

    TabPage *tabPage = new TabPage();
    ui->tabWidget->addTab(tabPage, input);
    ui->tabWidget->setCurrentWidget(tabPage);
    // 默认 tab 中有五项
    for(int i = 0; i < 5; i++){
        do_addItemToList();
    }
    connect(tabPage, &TabPage::insertItem, this, [=](int index){
        do_addItemToList(index, true);
    });
}

void MainWindow::do_addItemToList(int row, bool isInsert)
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(QSize(0, CUSTOM_ITEM_HEIGHT));
    CustomItem *addItem = new CustomItem();

    TabPage* tabPage = qobject_cast<TabPage*>(ui->tabWidget->currentWidget());
    QListWidget *listWidget = tabPage->getListWidget();
    if(isInsert){
        listWidget->insertItem(row, item);
    }
    else
        listWidget->addItem(item);
    listWidget->setItemWidget(item, addItem);

    connect(addItem, &CustomItem::sendDataRequest,this,&MainWindow::sendData);
}

void MainWindow::do_fileDownload(const SendFile_t &config)
{
    if(sendFile_.model == 0 && !serialManager_->isPortOpen_){
        QMessageBox::information(this,"提示","串口未开启");
        return;
    }
    else if(sendFile_.model == 1){
        QMessageBox::information(this,"提示","网络未开启");
        return;
    }
    else if(sendFile_.model == 2){
        QMessageBox::information(this,"提示","蓝牙未开启");
        return;
    }
    // 读出文件内容
    QFile file(config.filePath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::critical(this,"提示","文件打开失败");
        return;
    }
    // 阻塞式写法
    QByteArray content = file.readAll();
    file.close();

    // 先发送握手命令
    isFileDownload = true;
    fileReceiveBuffer_.clear();
    sendData(config.cmd);

    bool ackOK = waitAck(config.ack, config.timeoutMs);
    if (!ackOK) {
        QMessageBox::information(this,"提示","握手超时");
        isFileDownload = false;
        return;
    }

    int totalSize = content.size();
    int sent = 0;
    int packSize = config.dataSize;

    while (sent < totalSize) {
        // 取一包
        int len = qMin(packSize, totalSize - sent);
        QByteArray pack = content.mid(sent, len);

        // 发送（文件发送 → 不加校验）
        isFileDownload = true;
        serialManager_->sendData(pack);
        showSendData(pack);
        isFileDownload = false;

        // 等待 ACK
        if (!waitAck(config.ack, config.timeoutMs)) {
            labelInfoRefresh("等待 ACK 超时，发送失败");
            break;
        }
        sent += len;
    }
    isFileDownload = false;
    fileReceiveBuffer_.clear();

    QScrollBar *scrollBar = ui->plainTextEdit_Show->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::on_actDelTab_triggered()
{
    if(ui->tabWidget->count() == 1){
        QMessageBox::information(this,"提示","至少保留一页");
        return;
    }
    int index = ui->tabWidget->currentIndex();
    ui->tabWidget->removeTab(index);
    QMessageBox::information(this,"提示",QString("第%1页已删除").arg(index + 1));
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

// 双击修改 tab 页面名称
void MainWindow::on_tabWidget_tabBarDoubleClicked(int index)
{
    bool isOk = false;
    // 获取当前名称
    QString cur = ui->tabWidget->tabText(index);
    QString input = QInputDialog::getText(this,"输入","修改页面名称:",
                                          QLineEdit::Normal, cur,&isOk);
    if(!isOk || input.isEmpty()) return;

    ui->tabWidget->setTabText(index, input);
}

void MainWindow::on_tabWidget_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->tabWidget->mapToGlobal(pos);

    QMenu menu;
    menu.addAction(ui->actDelTab);

    menu.exec(globalPos);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    // 保存配置
    Config_t config;
    config.windowSize = this->size();
    config.baudRateIndex = ui->cbBox_PortBuad->currentIndex();
    config.check = (CheckDataIndex)ui->cbBox_DataCheck->currentIndex();
    config.filePath = fileSavePath_;
    config.isDockVisible = ui->dockWidget->isVisible();
    config.font = ui->plainTextEdit_Show->font();

    appConfig_->saveConfig(config);

    appConfig_->saveTabPage(ui->tabWidget);
}
