#ifndef CUSTOMDATASENDWIDGET_H
#define CUSTOMDATASENDWIDGET_H

#include "dataStructure.h"

#include <QWidget>

namespace Ui {
class CustomDataSendWidget;
}

class CustomDataSendWidget : public QWidget
{
    Q_OBJECT

private:
    Ui::CustomDataSendWidget *ui;

public:
    explicit CustomDataSendWidget(QWidget *parent = nullptr);
    ~CustomDataSendWidget();

    void setRemark(const QString remark);
    void setContent(const QString content);
    void setModel(const SendModel model);

    QString getRemark();
    QString getContent();
    SendModel getModel();

private slots:
    void lineEditSend();

signals:
    void sendDataRequest(const QString &content, SendModel sendmodel);

};

#endif // CUSTOMDATASENDWIDGET_H
