#include "sendw25qxx.h"
#include "ui_sendw25qxx.h"

#include <QFileDialog>
#include <QFile>
#include <QtMath>

// spinBox_flashPageSize 可以用继承 stepBy 实现指数步长

SendW25Qxx::SendW25Qxx(const SendW25Qxx_t &config, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::SendW25Qxx)
{
  ui->setupUi(this);
  this->setFixedSize(320, 450);
  this->setWindowTitle("文件传输");
  qDebug() << "width =" << this->width() << "height =" << this->height();

  ui->lineEdit_ack->setMaxLength(2);

  // config = config;

  if(config.model == 0)
    ui->label_sendModel->setText("串口传输");
  else if(config.model == 1)
    ui->label_sendModel->setText("网络传输");
  else if(config.model == 2)
    ui->label_sendModel->setText("蓝牙传输");

  // 配置显示
  ui->lineEdit_filePath->setText(config.filePath);
  ui->spinBox_flashPageSize->setValue(config.flashSize);
  ui->spinBox_flashPageIdx->setValue(config.flashIdx);
  ui->lineEdit_cmd->setText(config.cmd);
  ui->lineEdit_ack->setText(config.ack);
  ui->spinBox_timeout->setValue(config.timeoutMs);

  // 数据显示
  labelDataRefresh(config.filePath);

  // this->setToolTip("下载时请勿操作");
}

SendW25Qxx::~SendW25Qxx()
{
  delete ui;
}

SendW25Qxx_t SendW25Qxx::getConfig() const
{
  SendW25Qxx_t cfg = {0};

  cfg.filePath = ui->lineEdit_filePath->text();
  cfg.flashSize = ui->spinBox_flashPageSize->value();
  cfg.flashIdx = ui->spinBox_flashPageIdx->value();
  cfg.cmd = ui->lineEdit_cmd->text();
  cfg.ack = ui->lineEdit_ack->text();
  cfg.timeoutMs = ui->spinBox_timeout->value();

  return cfg;
}

QProgressBar *SendW25Qxx::getProgress()
{
  return ui->progressBar;
}

void SendW25Qxx::on_btn_openFile_clicked()
{
  QString name = QFileDialog::getOpenFileName(this, "选择文件", 
    ui->lineEdit_filePath->text(), "二进制文件 (*.bin)"
  );

  if(name.isEmpty()) return;

  ui->lineEdit_filePath->setText(name);
  labelDataRefresh(name);
}

void SendW25Qxx::labelDataRefresh(QString filePath)
{
  // 计算 bin 文件大小
  QFile file(filePath);

  if(file.open(QIODeviceBase::ReadOnly)) {
    fileSize_ = file.size();
    file.close();
  }

  ui->label_fileSize->setText(QString("%1B").arg(fileSize_));

  pageCntRefresh(fileSize_, ui->spinBox_flashPageSize->value());

  sectorRangeRefresh(ui->spinBox_flashPageIdx->value());
}

void SendW25Qxx::pageCntRefresh(int flashSize, int pageSize)
{
  double pageCnt = flashSize / (double)pageSize;

  ui->label_flashPageCnt->setText(QString::number(qCeil(pageCnt)));

  pageCntInSector_ = FLASH_SECTOR_SIZE / pageSize;
  // qDebug()<<pageCntInSector_;
}

void SendW25Qxx::sectorRangeRefresh(int basePage)
{
  double cnt = (double)fileSize_ / FLASH_SECTOR_SIZE;

  ui->label_sectorRange->setText(QString::number(qCeil(cnt)));

  int pageCnt = ui->label_flashPageCnt->text().toInt();
  int sumPage = basePage + pageCnt;
  qDebug()<<sumPage;
}

void SendW25Qxx::on_btn_transmit_clicked()
{
  emit tranmit(getConfig());
}

void SendW25Qxx::on_spinBox_flashPageSize_valueChanged(int i)
{
  pageCntRefresh(fileSize_, i);
}

void SendW25Qxx::on_spinBox_flashPageIdx_valueChanged(int i)
{
  sectorRangeRefresh(i);
}
