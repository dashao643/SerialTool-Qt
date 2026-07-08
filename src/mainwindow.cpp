#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog/serialsettingdialog.h"

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
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  this->setObjectName("MainWindow");
  ui->btn_PortRefresh->setText("");
  ui->btn_OpenClose->setProperty("connected", false);
  qDebug() << "width =" << this->width() << "height =" << this->height();
  this->setWindowTitle(QString("串口工具-v%1").arg(APP_VERSION));

  cbBoxInit();
  dataInit();
  uiInit();
  slotsInit();

  serialManager_->setBuadRate(ui->cbBox_PortBuad->currentText().toInt());
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::cbBoxInit()
{
  ui->cbBox_PortNum->setView(new QListView);
  ui->cbBox_Model->setView(new QListView);
  ui->cbBox_PortBuad->setView(new QListView);
  ui->cbBox_DataCheck->setView(new QListView);
  ui->cbBox_Network->setView(new QListView);

  // setView 之前 setupUi 已加载的静态 item，新 QListView 布局不到 → 清掉重加
  ui->cbBox_Model->clear();
  ui->cbBox_DataCheck->clear();
  ui->cbBox_Network->clear();
  ui->cbBox_Model->addItems({"串口配置", "网络配置", "蓝牙配置"});
  ui->cbBox_DataCheck->addItems({"None", "ModbusCRC16", "ADD8"});
  ui->cbBox_Network->addItems({"TCP服务端", "TCP客户端", "UDP"});
}

void MainWindow::dataInit()
{
  appConfig_ = new AppConfig(this);
  serialManager_ = new SerialManager(this);
  networkManager_ = new NetworkManager(this);

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
  // 初始化本地IP
  networkManager_->ipInit(ui->cbBox_LocalIP);
  ui->tabWidget->clear();
  loadPageConfig();
  ui->tabWidget->setCurrentIndex(0);
  ui->stackedWidget->setCurrentIndex(0);
  ui->cbBox_Model->setCurrentIndex(0);
  do_UiUpdate(false);
  // 加载配置
  Config_t config = appConfig_->loadConfig();
  sendFile_ = config.sendFile;
  sendW25Q_ = config.sendW25Q;
  // 执行后窗口不在桌面正中心
  // this->resize(config.windowSize);
  ui->cbBox_PortBuad->setCurrentIndex(config.baudRateIndex);
  ui->cbBox_DataCheck->setCurrentIndex(config.check);
  fileSavePath_ = config.filePath;
  ui->plainTextEdit_Show->setFont(config.font);
  ui->dockWidget->setVisible(config.isDockVisible);
  ui->lineEdit_LocalPort->setText(config.localPort);
  ui->lineEdit_RemoteIP->setText(config.remoteIP);
  ui->lineEdit_RemotePort->setText(config.remotePort);
}

void MainWindow::slotsInit()
{
  // 标签栏提示
  connect(infoTimer_,&QTimer::timeout,ui->label_InfoL,&QLabel::clear);
  /**************************** serialManager ****************************/
  connect(serialManager_,&SerialManager::sgn_labelShowInfo,this,&MainWindow::labelInfoRefresh);
  // 消息框提示
  connect(serialManager_,&SerialManager::sgn_showMessage,[=](const QString &string){
    QMessageBox::information(this, "提示", string);
  });
  // ui更新
  connect(serialManager_,&SerialManager::sgn_serialStateChanged,this,&MainWindow::do_UiUpdate);
  // 串口开关按键状态
  connect(serialManager_,&SerialManager::sgn_availablePort,[=](bool isEmpty){
    ui->btn_OpenClose->setDisabled(isEmpty);
  });
  // 串口有数据 -> 串口读数据
  connect(serialManager_,&SerialManager::sgn_readyRead,this,&MainWindow::do_readyRead);
  // 刷新端口
  connect(ui->btn_PortRefresh,&QPushButton::clicked,serialManager_,[=](){
    serialManager_->do_btnPortRefresh(ui->cbBox_PortNum);
  });
  /**************************** networkManager ****************************/
  connect(networkManager_,&NetworkManager::sgn_stateChange,this,&MainWindow::do_netWorkStateUpdate);
  connect(networkManager_,&NetworkManager::sgn_readyRead,this,&MainWindow::do_readyRead);
  connect(networkManager_,&NetworkManager::sgn_btnStateChanged,this,&MainWindow::do_UiUpdate);
  connect(networkManager_,&NetworkManager::sgn_labelShowState,this,[=](const QString& text){
    ui->label_InfoR->setText(text);
  });
  connect(networkManager_,&NetworkManager::sgn_showMessage,[=](const QString &string){
    QMessageBox::information(this, "提示", string);
  });
  // 选择模式
  connect(ui->cbBox_Model,&QComboBox::currentIndexChanged,this,[=](int index){
    // 切换模式时关闭所有连接，避免冲突
    serialManager_->closeConnection();
    networkManager_->closeConnection();
    // 界面显示变更
    ui->stackedWidget->setCurrentIndex(index);
  });
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
    connect(&dialog,&SendFileDialog::download,this,[=, &dialog](const SendFile_t &cfg){
      do_fileDownload(cfg, &dialog);
    });
    dialog.exec();
    isFileDownload = false;
    sendFile_ = dialog.getConfig();
  });
  connect(ui->actSendW25Q,&QAction::triggered,this,[=](){
    sendW25Q_.model = ui->cbBox_Model->currentIndex();
    SendW25Qxx dialog(sendW25Q_, this);
    connect(&dialog, &SendW25Qxx::tranmit, this, [=, &dialog](const SendW25Qxx_t &cfg){
      do_fileDownload(cfg, &dialog);
    });
    dialog.exec();
    isFileDownload = false;
    sendW25Q_ = dialog.getConfig();
  });
  // 波特率修改
  connect(ui->cbBox_PortBuad,&QComboBox::currentIndexChanged,serialManager_,[=](){
    serialManager_->setBuadRate(ui->cbBox_PortBuad->currentText().toInt());
  });
  // TCP服务端禁用远程ip设置。UDP禁用本地IP设置
  connect(ui->cbBox_Network,&QComboBox::currentIndexChanged,[=](int index){
    ui->lineEdit_RemoteIP->setDisabled(index == TcpServer);
    ui->lineEdit_RemotePort->setDisabled(index == TcpServer);
    ui->cbBox_LocalIP->setDisabled(index == UDP);
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
    // QListWidget *list = tabPage->getListWidget();
    for(int i = 0; i < page.items.count(); i++){
      CustomItem* customItem = do_addItemToList();
      ItemConfig& cfg = page.items[i];
      customItem->setRemark(cfg.remark);
      customItem->setContent(cfg.content);
      customItem->setModel(cfg.model);
    }
  }
}

// 信号槽绑定的参数，要么传值，要么传 const + 引用
void MainWindow::sendData(QString content, SendModel model)
{
  if(content.isEmpty()){
    labelInfoRefresh("发送内容不能为空");
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
    if (content.isEmpty()) {
      QMessageBox::information(this,"提示","请输入合法Hex数");
      return;
    }
    sendBuf = QByteArray::fromHex(content.toUtf8());
    appendCheckData(sendBuf);
  }
  // 串口发送
  if(ui->cbBox_Model->currentIndex() == SERIAL){
    serialManager_->sendData(sendBuf);
  }
  // 网络发送
  else if(ui->cbBox_Model->currentIndex() == NETWORK){
    networkManager_->sendData(sendBuf);
  }
  if(!isConnected_)
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
  ui->label_InfoL->setText(strInfo);
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
    // 让界面 & 串口接收正常运行
    QCoreApplication::processEvents();

    // 检查是否收到正确 ACK
    if (fileReceiveBuffer_.contains(targetAck)) {
      fileReceiveBuffer_.clear();
      return true;
    }
  }
  return false;
}

// 更新所有有关串口开关状态的显示
void MainWindow::do_UiUpdate(bool isOpen)
{
  isConnected_ = isOpen;
  if(isOpen){
    ui->btn_OpenClose->setText("关闭");
    if(ui->cbBox_Model->currentIndex() == ComModel::SERIAL)
      ui->label_InfoR->setText("当前状态：串口开启");
    else if(ui->cbBox_Model->currentIndex() == ComModel::NETWORK &&
                 ui->cbBox_Network->currentIndex() == 0){
      ui->label_InfoR->setText("网络状态：服务器监听中...");
    }
    ui->btn_OpenClose->setProperty("connected", true);
  }
  else{
    ui->btn_OpenClose->setText("开启");
    ui->label_InfoR->setText("当前状态：关闭");
    ui->btn_OpenClose->setProperty("connected", false);
  }

  // Force QSS re-evaluation for the dynamic property change
  ui->btn_OpenClose->style()->unpolish(ui->btn_OpenClose);
  ui->btn_OpenClose->style()->polish(ui->btn_OpenClose);
  ui->actPortSetting->setDisabled(isOpen);
  ui->cbBox_PortNum->setDisabled(isOpen);
  ui->btn_Send->setEnabled(isOpen);
}

void MainWindow::on_btn_OpenClose_clicked()
{
  if(ui->cbBox_Model->currentIndex() == ComModel::SERIAL)
    serialManager_->do_btnOpenClose(ui->cbBox_PortNum);
  else if(ui->cbBox_Model->currentIndex() == ComModel::NETWORK){
    CurNetworkModel model = (CurNetworkModel)ui->cbBox_Network->currentIndex();
    QString localIp = "", targetIp = "";
    quint16 localPort = 0, targetPort = 0;
    // TCP服务端，传本地ip和端口
    if(model == TcpServer){
      localIp = ui->cbBox_LocalIP->currentText();
      localPort = ui->lineEdit_LocalPort->text().toInt();
      networkManager_->do_btnOpenClose(model, localIp, localPort);
    }
    // CP客户端传远程目标ip和端口;
    else if(model == TcpClient){
      targetIp = ui->lineEdit_RemoteIP->text();
      targetPort = ui->lineEdit_RemotePort->text().toInt();
      networkManager_->do_btnOpenClose(model, targetIp, targetPort);
    }
    // UDP传本地端口，目标ip和端口
    else if(model == UDP){
      networkManager_->do_btnOpenClose(
        ui->lineEdit_LocalPort->text().toInt(),
        ui->lineEdit_RemoteIP->text(),
        ui->lineEdit_RemotePort->text().toInt()
      );
    }
  }
}

void MainWindow::do_netWorkStateUpdate(QAbstractSocket::SocketState state)
{
  switch(state)
  {
  case QAbstractSocket::UnconnectedState:
    if(ui->cbBox_Network->currentIndex() == 0)
      ui->label_InfoR->setText("网络状态：已断开");
    else if(ui->cbBox_Network->currentIndex() == 1)
      ui->label_InfoR->setText("网络状态：连接失败");
    break;
  case QAbstractSocket::ConnectedState:
    ui->label_InfoR->setText("网络状态：已连接");           break;
  case QAbstractSocket::ClosingState:
    ui->label_InfoR->setText("网络状态：正在关闭...");         break;
  case QAbstractSocket::ListeningState:
    ui->label_InfoR->setText("网络状态：监听中...");       break;
  case QAbstractSocket::HostLookupState:
    ui->label_InfoR->setText("网络状态：正在解析主机域名...");  break;
  case QAbstractSocket::ConnectingState:
    ui->label_InfoR->setText("连接中...");                break;
  case QAbstractSocket::BoundState:
    break;
  }
}

void MainWindow::do_readyRead(const QByteArray &byteArray)
{
  receiveBuffer_.append(byteArray);
  rxTimer_->start(ui->spBox_Timeout->value());

  if (isFileDownload) {
    fileReceiveBuffer_.append(byteArray);
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
  // HEX按空格/换行分割，统计有效数据个数
  int size = 0;
  if(ui->rdBtn_ShowHex->isChecked()){
    QStringList list = selectedText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    size = list.size();
  }
  else if(ui->rdBtn_ShowASCII->isChecked()){
    size = selectedText.size();
  }
  labelInfoRefresh(QString("当前选中个数：%1").arg(size));
}

// 根据当前选择的模式发送数据
void MainWindow::on_btn_Send_clicked()
{
  QString content = ui->plainTextEdit_Send->toPlainText();
  if(content.isEmpty()){
    labelInfoRefresh("发送内容不能为空");
    return;
  }
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

  QByteArray binData = QByteArray::fromHex(selected.toUtf8());
  QString ascii = QString::fromLocal8Bit(binData);

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
                     QLineEdit::Normal, defaultName, &isOk);
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

CustomItem* MainWindow::do_addItemToList(int row, bool isInsert)
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

  return addItem;
}

void MainWindow::do_fileDownload(const SendFile_t &config, SendFileDialog* dialog)
{
  if(sendFile_.model == 0 && !serialManager_->isOpen()){
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
  QByteArray content = file.readAll();
  file.close();
  QProgressBar *progressBar = dialog->getProgress();
  progressBar->setMaximum(content.size());

  // 阻塞式写法。先发送握手命令，附带校验
  fileReceiveBuffer_.clear();
  isFileDownload = true;
  sendData(config.cmd, HEX);
  bool ackOK = waitAck(config.ack, config.timeoutMs);
  if (!ackOK) {
    QMessageBox::critical(this,"提示","握手超时，升级失败");
    return;
  }

  int totalSize = content.size();
  int sent = 0;
  int packSize = config.dataSize;

  while (sent < totalSize) {
    int len = qMin(packSize, totalSize - sent);
    QByteArray pack = content.mid(sent, len);
    // 最后一包长度不足，用 0xFF 填充
    if (pack.size() < packSize) {
      pack.append(packSize - pack.size(), (uint8_t)0xFF);
    }
    serialManager_->sendData(pack);
    showSendData(pack);

    if (!waitAck(config.ack, config.timeoutMs)) {
      QMessageBox::critical(this,"提示","ACK超时, 升级失败");
      return;
    }
    sent += len;
    progressBar->setValue(sent);
  }
  QMessageBox::information(this,"完成","文件发送成功!");
}

void MainWindow::do_fileDownload(const SendW25Qxx_t &sendFile, SendW25Qxx *dialog)
{
  
}

void MainWindow::on_actDelTab_triggered()
{
  if(ui->tabWidget->count() == 1){
    QMessageBox::information(this,"提示","至少保留一页");
    return;
  }
  int index = ui->tabWidget->currentIndex();
  QWidget *w = ui->tabWidget->widget(index);
  ui->tabWidget->removeTab(index);
  delete w;
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
  // 保存配置
  Config_t config;
  config.windowSize = this->size();
  config.baudRateIndex = ui->cbBox_PortBuad->currentIndex();
  config.check = (CheckDataIndex)ui->cbBox_DataCheck->currentIndex();
  config.filePath = fileSavePath_;
  config.isDockVisible = ui->dockWidget->isVisible();
  config.font = ui->plainTextEdit_Show->font();
  config.sendFile = sendFile_;
  config.sendW25Q = sendW25Q_;
  config.localPort = ui->lineEdit_LocalPort->text();
  config.remoteIP = ui->lineEdit_RemoteIP->text();
  config.remotePort = ui->lineEdit_RemotePort->text();
  appConfig_->saveConfig(config);

  appConfig_->saveTabPage(ui->tabWidget);

  QMainWindow::closeEvent(event);
}

void MainWindow::on_toolBar_customContextMenuRequested(const QPoint &pos)
{
  QString tabName = ui->tabWidget->tabText(0);
  ui->actTimeSyn->setEnabled(tabName == "modbus");

  QPoint globalPos = ui->toolBar->mapToGlobal(pos);
  QMenu menu;
  menu.addAction(ui->actTimeSyn);
  menu.exec(globalPos);
}

void MainWindow::on_actTimeSyn_triggered()
{
  auto toHex2 = [](int val) {
    return QString("%1").arg(val, 2, 16, QChar('0')).toUpper();
  };

  QDate curDate = QDate::currentDate();
  QTime curTime = QTime::currentTime();

  QString year   = toHex2(curDate.year() % 100);
  QString month  = toHex2(curDate.month());
  QString day    = toHex2(curDate.day());
  QString hour   = toHex2(curTime.hour());
  QString minute = toHex2(curTime.minute());
  QString second = toHex2(curTime.second());

  // 从机地址 + 指令码 + 寄存器地址 + 寄存器数量 + 字节数
  sendData("01 10 FF FF 00 06 06" + year + month + day + hour + minute + second, HEX);
}
