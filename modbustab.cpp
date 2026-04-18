#include "modbustab.h"

ModbusTab::ModbusTab(QObject *parent, QString slaveAddress)
    : QObject{parent}
    , m_slaveAddress(slaveAddress)
{

}

void ModbusTab::setSlaveAddress(QString addr) { m_slaveAddress = addr; }

QString ModbusTab::LED_RED_ON()       { return m_slaveAddress + " 05 00 01 00 01 01"; }
QString ModbusTab::LED_RED_OFF()      { return m_slaveAddress + " 05 00 01 00 01 00"; }
QString ModbusTab::LED_RED_TOGGLE()   { return m_slaveAddress + " 05 00 01 00 01 02"; }

QString ModbusTab::LED_GREEN_ON()     { return m_slaveAddress + " 05 00 02 00 01 01"; }
QString ModbusTab::LED_GREEN_OFF()    { return m_slaveAddress + " 05 00 02 00 01 00"; }
QString ModbusTab::LED_GREEN_TOGGLE() { return m_slaveAddress + " 05 00 02 00 01 02"; }

QString ModbusTab::LED_BLUE_ON()      { return m_slaveAddress + " 05 00 03 00 01 01"; }
QString ModbusTab::LED_BLUE_OFF()     { return m_slaveAddress + " 05 00 03 00 01 00"; }
QString ModbusTab::LED_BLUE_TOGGLE()  { return m_slaveAddress + " 05 00 03 00 01 02"; }

QString ModbusTab::DHT11_READ_TEMP()  { return m_slaveAddress + " 04 00 04 00 01"; }
QString ModbusTab::DHT11_READ_HUMI()  { return m_slaveAddress + " 04 00 05 00 01"; }
QString ModbusTab::DHT11_READ_TH()    { return m_slaveAddress + " 04 00 04 00 02"; }

QString ModbusTab::RTC_SET_TIME()      { return m_slaveAddress + " 06 00 07 00 01"; }
QString ModbusTab::RTC_GET_TIME()      { return m_slaveAddress + " 04 00 07 00 01"; }
