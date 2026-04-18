#ifndef MODBUSTAB_H
#define MODBUSTAB_H

#include <QObject>
#include <QString>

typedef enum{
    LED_RED = 1,
    LED_GREEN,
    LED_BLUE
}LED;

typedef enum{
    LED_OFF = 0,
    LED_ON,
    LED_TOGGLE
}LED_WRITE_MODEL;

typedef enum{
    DHT11_TEMP = 0,
    DHT11_HUMI,
    DHT11_TH
}DHT11;

typedef enum{
    RTC_TIME = 0,
}RTC;

class ModbusTab : public QObject
{
    Q_OBJECT
public:
    explicit ModbusTab(QObject *parent = nullptr, QString slaveAddress = "00");

    void setSlaveAddress(QString addr);
    // QString getSlaveAddress() const;

    // QString LED_Control(LED led, LED_WRITE_MODEL model);
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
    QString add;
};

#define MODBUS_FUNC_READ_COILS          0x01    // 读线圈（输出IO）
#define MODBUS_FUNC_READ_DISCRETE_INPUT 0x02    // 读离散输入（输入IO）
#define MODBUS_FUNC_READ_HOLD_REGS      0x03    // 读保持寄存器（可读可写变量）
#define MODBUS_FUNC_READ_INPUT_REGS     0x04    // 读输入寄存器（只读变量）       // 支持
#define MODBUS_FUNC_WRITE_SINGLE_COIL   0x05    // 写单个线圈                    // 支持
#define MODBUS_FUNC_WRITE_SINGLE_REG    0x06    // 写单个保持寄存器
#define MODBUS_FUNC_WRITE_MULTI_COILS   0x0F    // 写多个线圈
#define MODBUS_FUNC_WRITE_MULTI_REGS    0x10    // 写多个保持寄存器

#endif // MODBUSTAB_H
