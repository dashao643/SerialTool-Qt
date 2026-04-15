#ifndef SERIALSETTINGDIALOG_H
#define SERIALSETTINGDIALOG_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
class SerialSettingDialog;
}

class SerialSettingDialog : public QDialog
{
    Q_OBJECT

public:
    Ui::SerialSettingDialog *ui;
    QSerialPort::DataBits m_dataBits;            // 数据位
    QSerialPort::StopBits m_stopBits;            // 停止位
    QSerialPort::Parity m_parity;                // 校验位

    explicit SerialSettingDialog(QWidget *parent = nullptr);
    ~SerialSettingDialog();

    void comboBoxInit();

private slots:
    void on_buttonBox_accepted();

private:

};

#endif // SERIALSETTINGDIALOG_H
