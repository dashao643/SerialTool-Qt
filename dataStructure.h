#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <QSerialPort>
#include <QSize>
#include <QFont>

// 公共数据结构定义

typedef enum{
    SERIAL = 0,
    NETWORK,
    BLUETOOTH
}ComModel;

typedef enum{
    HEX = 0,
    ASCII,
}SendModel;

typedef enum{
    NONE_CHECK = 0,
    MODBUS_CRC16,
    ADD8,
}CheckDataIndex;

typedef struct{
    QSerialPort::DataBits    dataBits;
    QSerialPort::StopBits    stopBits;
    QSerialPort::Parity      parity;
    QSerialPort::FlowControl flowControl;
}SerialConfigStruct;

typedef struct
{
    QString remark;
    QString content;
    SendModel model;
}ItemConfig;

typedef struct{
    QString filePath;
    int dataSize;
    QString cmd;
    QString ack;
    int timeoutMs;
    int model;
} SendFile_t;

typedef struct
{
    QString name;
    QList<ItemConfig> items;
}TabPageConfig;

typedef struct{
    QSize windowSize;
    int baudRateIndex;
    CheckDataIndex check;
    QString filePath;
    QFont font;
    bool isDockVisible;
    SendFile_t sendFile;
} Config_t;

#endif // DATASTRUCTURE_H
