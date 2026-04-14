#ifndef SERIALSETTINGDIALOG_H
#define SERIALSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SerialSettingDialog;
}

class SerialSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SerialSettingDialog(QWidget *parent = nullptr);
    ~SerialSettingDialog();

    Ui::SerialSettingDialog *ui;
private:

};

#endif // SERIALSETTINGDIALOG_H
