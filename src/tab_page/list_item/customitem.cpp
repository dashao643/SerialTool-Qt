#include "customitem.h"
#include "ui_customitem.h"

CustomItem::CustomItem(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::CustomItem)
{
  ui->setupUi(this);
}

CustomItem::~CustomItem()
{
  delete ui;
}

void CustomItem::setRemark(const QString remark)
{
  ui->lineEdit_Comment->setText(remark);
}

void CustomItem::setContent(const QString content)
{
  ui->lineEdit_Data->setText(content);
}

void CustomItem::setModel(const SendModel model)
{
  ui->checkBox_Hex->setChecked(model == SendModel::HEX);
}

QString CustomItem::getRemark()
{
  return ui->lineEdit_Comment->text();
}

QString CustomItem::getContent()
{
  return ui->lineEdit_Data->text();
}

SendModel CustomItem::getModel()
{
  return ui->checkBox_Hex->isChecked() ? SendModel::HEX : SendModel::ASCII;
}

QPushButton *CustomItem::getSendBtn()
{
  return ui->btn_Send;
}

void CustomItem::on_btn_Send_clicked()
{
  QString content = ui->lineEdit_Data->text();
  SendModel model = ui->checkBox_Hex->isChecked() ? SendModel::HEX : SendModel::ASCII;
  emit sendDataRequest(content, model);
}

