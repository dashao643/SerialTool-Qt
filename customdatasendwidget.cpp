#include "customdatasendwidget.h"
#include "ui_customdatasendwidget.h"

CustomDataSendWidget::CustomDataSendWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomDataSendWidget)
{
    ui->setupUi(this);
    connect(ui->btn_Send,&QPushButton::clicked,this,&CustomDataSendWidget::lineEditSend);
}

CustomDataSendWidget::~CustomDataSendWidget()
{
    delete ui;
}

void CustomDataSendWidget::setRemark(const QString remark)
{
    ui->lineEdit_Comment->setText(remark);
}

void CustomDataSendWidget::setContent(const QString content)
{
    ui->lineEdit_Data->setText(content);
}

void CustomDataSendWidget::setModel(const SendModel model)
{
    ui->checkBox_Hex->setChecked(model == SendModel::HEX);
}

QString CustomDataSendWidget::getRemark()
{
    return ui->lineEdit_Comment->text();
}

QString CustomDataSendWidget::getContent()
{
    return ui->lineEdit_Data->text();
}

SendModel CustomDataSendWidget::getModel()
{
    return ui->checkBox_Hex->isChecked() ? SendModel::HEX : SendModel::ASCII;
}

void CustomDataSendWidget::lineEditSend()
{
    QString content = ui->lineEdit_Data->text();
    SendModel model = ui->checkBox_Hex->isChecked() ? SendModel::HEX : SendModel::ASCII;
    emit sendDataRequest(content, model);
}
