#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <QSerialPort>

// 所有公共数据结构定义

typedef enum{
    HEX = 0,
    ASCII,
}SendModel;

typedef enum{
    NONE_CHECK = 0,
    MODBUS_CRC16,
    ADD8,
}CheckDataIndex;

typedef enum{
    CUSTOM = 0,
    MODBUS,
    ESP8266,
}SendProtocol;

typedef struct{
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity parity;
    QSerialPort::FlowControl flowControl;
}SerialConfigStruct;



#endif // DATASTRUCTURE_H
