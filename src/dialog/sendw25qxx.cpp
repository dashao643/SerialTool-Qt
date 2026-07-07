#include "sendw25qxx.h"
#include "ui_sendw25qxx.h"

#include <QPushButton>
#include <QFileDialog>

SendW25Qxx::SendW25Qxx(const SendW25Qxx_t &config, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::SendW25Qxx)
{
  ui->setupUi(this);
  this->setFixedSize(320, 360);
  this->setWindowTitle("文件传输");
  qDebug()<<this->width()<<" "<<this->height();

  ui->lineEdit_ack->setMaxLength(2);

  // config = config;

  if(config.model == 0)
    ui->label_sendModel->setText("串口传输");
  else if(config.model == 1)
    ui->label_sendModel->setText("网络传输");
  else if(config.model == 2)
    ui->label_sendModel->setText("蓝牙传输");

  // 初始化显示到UI
  ui->lineEdit_filePath->setText(config.filePath);
  ui->spinBox_flashPageSize->setValue(config.flashSize);
  ui->spinBox_flashPageIdx->setValue(config.flashIdx);
  ui->lineEdit_cmd->setText(config.cmd);
  ui->lineEdit_ack->setText(config.ack);
  ui->spinBox_timeout->setValue(config.timeoutMs);

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
  QString name = QFileDialog::getOpenFileName(this, "选择文件", ui->lineEdit_filePath->text(),
                                 "二进制文件 (*.bin)");
  if(name.isEmpty())
    return;
  ui->lineEdit_filePath->setText(name);
}

void SendW25Qxx::on_btn_transmit_clicked()
{
  emit tranmit(getConfig());
}
