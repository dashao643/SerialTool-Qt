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
    SerialConfigStruct serialConfig_;                      // 串口其他配置

    explicit SerialManager(QObject *parent = nullptr);

    void baudRateInit(QComboBox *comboBox);
    bool isOpen() const { return isPortOpen_; }
    void sendData(const QByteArray &content);
    void setBuadRate(int buadRate);
    void closeConnection();

public slots:
    void do_btnPortRefresh(QComboBox *comboBox);
    void do_btnOpenClose(QComboBox *comboBoxPort);

signals:
    void sgn_serialStateChanged(bool isOpen);
    void sgn_availablePort(bool isEmpty);
    void sgn_labelShowInfo(const QString& text);
    void sgn_showMessage(const QString& text);
    void sgn_readyRead(const QByteArray &byteArray);

private:
    QSerialPort *serialPort_;
    bool isPortOpen_ = false;

    void dataInit();
    void slotsInit();
    void comPortSetting();

private slots:
};

#endif // SERIALMANAGER_H
