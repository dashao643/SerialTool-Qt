#ifndef CUSTOMDATASENDWIDGET_H
#define CUSTOMDATASENDWIDGET_H

#include <QWidget>

namespace Ui {
class CustomDataSendWidget;
}

class CustomDataSendWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomDataSendWidget(QWidget *parent = nullptr);
    ~CustomDataSendWidget();

private slots:


private:
    Ui::CustomDataSendWidget *ui;
};

#endif // CUSTOMDATASENDWIDGET_H
