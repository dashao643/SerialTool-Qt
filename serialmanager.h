#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include "dataStructure.h"

#include <QObject>
#include <QSerialPort>
#include <QComboBox>
#include <QLabel>

class SerialManager : public QObject
{
    Q_OBJECT
public:
    QSerialPort *serialPort_;
    bool isPortOpen_ = false;
    SerialConfigStruct serialConfig_;                      // 串口其他配置

    explicit SerialManager(QObject *parent = nullptr);
    void dataInit();
    void slotsInit();
    void baudRateInit(QComboBox *comboBox);

    void sendData(const QByteArray &content);
    void setBuadRate(QComboBox *comboBox, int index);

public slots:
    void do_btnPortRefresh(QComboBox *comboBox);
    void do_btnOpenClose(QComboBox *comboBoxPort, const QComboBox *comboBoxBuadRate);

signals:
    void serialStateChanged(bool isOpen);
    void availablePort(bool isEmpty);
    void labelShowInfo(const QString& text);
    void showMessage(const QString& text);

private:
    void comPortSetting(const QComboBox *comboBoxBuadRate);

private slots:
};

#endif // SERIALMANAGER_H
