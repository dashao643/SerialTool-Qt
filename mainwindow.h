#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QTime>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QSerialPort *m_comPort = nullptr;                       // 打开的串口号
    QList<qint32> m_standardBaudRates;                      // 标准波特率列表
    qint8 m_defaultBaudRateIndex = 0;                       // 默认选择的波特率
    QList<QSerialPortInfo> m_curAvailablePorts;             // 当前可用串口列表
    QString m_curPortName;
    bool m_isPortOpen = false;
    QTimer *m_infoTimer = nullptr;
    QTime *curTime = nullptr;
    QByteArray m_receiveBuffer;                             // 接收缓冲区
    QTimer *m_rxTimer = nullptr;                            // 接收超时定时器
    // QFile iniFile;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void dataInit();
    void uiInit();
    void slotsInit();
    void iniFileInit();

private slots:
    void do_cbBoxPortNumRefresh();
    void do_btnOpenClose();
    void do_comReadyRead();
    void do_showReceivedData();
    void do_comSendData();

    void on_actPortSetting_triggered();
    void on_actClear_triggered();
    void on_actFont_triggered();
    void on_cbBox_PortBuad_currentIndexChanged(int index);

private:
    void comPortSetting();
    void showSendData(QByteArray sendBuf);
    void labelInfoRefresh(QString strInfo);

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
