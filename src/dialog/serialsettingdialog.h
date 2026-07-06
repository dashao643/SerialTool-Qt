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
  QSerialPort::DataBits m_dataBits;
  QSerialPort::StopBits m_stopBits;
  QSerialPort::Parity m_parity;
  QSerialPort::FlowControl m_flowControl;

  explicit SerialSettingDialog(QWidget *parent = nullptr);
  ~SerialSettingDialog();

  void comboBoxInit();

private slots:
  void on_buttonBox_accepted();

private:
  Ui::SerialSettingDialog *ui;
};

#endif // SERIALSETTINGDIALOG_H
