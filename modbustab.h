#ifndef MODBUSTAB_H
#define MODBUSTAB_H

#include <QObject>
#include <QString>

class ModbusTab : public QObject
{
    Q_OBJECT
public:
    explicit ModbusTab(QObject *parent = nullptr, QString slaveAddress = "00");

    void setSlaveAddress(QString addr);
    QString getSlaveAddress() const;

    // 指令全部变成函数，动态拼接！
    QString LED_RED_ON();
    QString LED_RED_OFF();
    QString LED_RED_TOGGLE();

    QString LED_GREEN_ON();
    QString LED_GREEN_OFF();
    QString LED_GREEN_TOGGLE();

    QString LED_BLUE_ON();
    QString LED_BLUE_OFF();
    QString LED_BLUE_TOGGLE();

    QString DHT11_READ_TEMP();
    QString DHT11_READ_HUMI();
    QString DHT11_READ_TH();

    QString RTC_SET_TIME();
    QString RTC_GET_TIME();

private:
    QString m_slaveAddress;
};

#endif // MODBUSTAB_H
