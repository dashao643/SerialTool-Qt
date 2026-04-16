#include "customdatasendwidget.h"
#include "ui_customdatasendwidget.h"

CustomDataSendWidget::CustomDataSendWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomDataSendWidget)
{
    ui->setupUi(this);
}

CustomDataSendWidget::~CustomDataSendWidget()
{
    delete ui;
}


