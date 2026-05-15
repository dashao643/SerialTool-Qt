#include "sendfiledialog.h"
#include "ui_sendfiledialog.h"

#include <QPushButton>
#include <QFileDialog>

SendFileDialog::SendFileDialog(SendFile_t sendFileConfig, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SendFileDialog)
{
    ui->setupUi(this);
    this->setFixedSize(size());
    this->setWindowTitle("文件传输");
    qDebug()<<this->width()<<" "<<this->height();

    ui->lineEdit_Ack->setMaxLength(2);

    config_ = sendFileConfig;
    if(config_.model == 0)
        ui->label_SendModel->setText("串口传输");
    else if(config_.model == 1)
        ui->label_SendModel->setText("网络传输");
    else if(config_.model == 2)
        ui->label_SendModel->setText("蓝牙传输");
    // 初始化显示到UI
    ui->lineEdit_FilePath->setText(config_.filePath);
    ui->spinBox_DataSize->setValue(config_.dataSize);
    ui->lineEdit_Cmd->setText(config_.cmd);
    ui->lineEdit_Ack->setText(config_.ack);
    ui->spinBox_Timeout->setValue(config_.timeoutMs);

    // this->setToolTip("下载时请勿操作");
}

SendFileDialog::~SendFileDialog()
{
    delete ui;
}

SendFile_t SendFileDialog::getConfig() const
{
    SendFile_t cfg = config_;

    cfg.filePath = ui->lineEdit_FilePath->text();
    cfg.dataSize = ui->spinBox_DataSize->value();
    cfg.cmd     = ui->lineEdit_Cmd->text();
    cfg.ack     = ui->lineEdit_Ack->text();
    cfg.timeoutMs = ui->spinBox_Timeout->value();

    return cfg;
}

QProgressBar *SendFileDialog::getProgress()
{
    return ui->progressBar;
}

void SendFileDialog::on_btn_OpenFile_clicked()
{
    QString name = QFileDialog::getOpenFileName(this, "选择文件", config_.filePath,
                                 "二进制文件 (*.bin)");
    ui->lineEdit_FilePath->setText(name);
}

void SendFileDialog::on_btn_Download_clicked()
{
    emit download(getConfig());
}
