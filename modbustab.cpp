#include "modbustab.h"

#include <QDate>
#include <QTime>

ModbusTab::ModbusTab(QObject *parent, QString slaveAddress)
    : QObject{parent}
    , add(slaveAddress)
{

}

void ModbusTab::setSlaveAddress(QString addr) { add = addr; }

QString ModbusTab::LED_RED_ON()       { return add + " 05 00 01 00 01 01"; }
QString ModbusTab::LED_RED_OFF()      { return add + " 05 00 01 00 01 00"; }
QString ModbusTab::LED_RED_TOGGLE()   { return add + " 05 00 01 00 01 02"; }

QString ModbusTab::LED_GREEN_ON()     { return add + " 05 00 02 00 01 01"; }
QString ModbusTab::LED_GREEN_OFF()    { return add + " 05 00 02 00 01 00"; }
QString ModbusTab::LED_GREEN_TOGGLE() { return add + " 05 00 02 00 01 02"; }

QString ModbusTab::LED_BLUE_ON()      { return add + " 05 00 03 00 01 01"; }
QString ModbusTab::LED_BLUE_OFF()     { return add + " 05 00 03 00 01 00"; }
QString ModbusTab::LED_BLUE_TOGGLE()  { return add + " 05 00 03 00 01 02"; }

QString ModbusTab::DHT11_READ_TEMP()  { return add + " 04 00 04 00 01"; }
QString ModbusTab::DHT11_READ_HUMI()  { return add + " 04 00 05 00 01"; }
QString ModbusTab::DHT11_READ_TH()    { return add + " 04 00 04 00 02"; }

QString ModbusTab::RTC_SET_TIME(){
    auto toHex2 = [](int val) {
        return QString("%1").arg(val, 2, 16, QChar('0')).toUpper();
    };
    QString year   = toHex2(QDate::currentDate().year() % 100);
    QString month  = toHex2(QDate::currentDate().month());
    QString day    = toHex2(QDate::currentDate().day());
    QString hour   = toHex2(QTime::currentTime().hour());
    QString minute = toHex2(QTime::currentTime().minute());
    QString second = toHex2(QTime::currentTime().second());
    qDebug()<<year << month << day;
    qDebug()<<hour << minute << second;
    return add + QString("10 00 01 00 06 06 %1 %2 %3 %4 %5 %6")
                     .arg(year).arg(month).arg(day).arg(hour).arg(minute).arg(second);
}

QString ModbusTab::RTC_GET_TIME(){
    return add + " 04 00 07 00 01";
}
