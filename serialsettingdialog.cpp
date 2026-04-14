#include "serialsettingdialog.h"
#include "ui_serialsettingdialog.h"

SerialSettingDialog::SerialSettingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SerialSettingDialog)
{
    ui->setupUi(this);
    this->setFixedSize(250,250);
    this->setWindowTitle("串口其它参数配置");
}

SerialSettingDialog::~SerialSettingDialog()
{
    delete ui;
}
