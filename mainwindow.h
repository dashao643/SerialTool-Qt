#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dataStructure.h"
#include "modbustab.h"

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QTime>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    const QString INI_FILE_NAME = "serialTool.ini";
    static constexpr int DEFAULT_WINDOW_WIDTH = 1024;
    static constexpr int DEFAULT_WINDOW_HEIGHT = 800;

    QSerialPort *m_comPort = nullptr;                       // 打开的串口号
    QList<QSerialPortInfo> m_curAvailablePorts;             // 当前可用串口列表
    QList<qint32> m_standardBaudRates;                      // 标准波特率列表
    QString m_curPortName;
    bool m_isPortOpen = false;

    QByteArray m_receiveBuffer;                             // 接收缓冲区
    QSettings *m_settings;

    QTimer *m_infoTimer = nullptr;
    QTimer *m_rxTimer = nullptr;                            // 接收超时定时器
    QTimer *m_sendTimer;                                    // 发送定时器

    /// 工具配置
    qint8 m_baudRateIndex = 0;                              // 波特率索引项
    SerialConfigStruct m_serialConfig;                      // 串口其他配置
    // QString m_saveTextFilePath;                             // 默认保存的文件路径

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void dataInit();
    void uiInit();
    void slotsInit();
    void loadConfig();

    int num = 0;
private slots:
    void do_cbBoxPortNumRefresh();
    void do_btnOpenClose();
    void do_comReadyRead();
    void do_showReceivedData();
    void do_btnComSendData();
    void do_calSelCharCnt();

    void on_actPortSetting_triggered();
    void on_actClear_triggered();
    void on_actSave_triggered();
    void on_actFont_triggered();
    void on_actBackgroundSetting_triggered();
    void on_actDockFloat_triggered(bool checked);
    void on_actDockVisible_triggered(bool checked);
    void on_actAdd_triggered();
    void on_actInsert_triggered();
    void on_actDel_triggered();

    void on_cbBox_PortBuad_currentIndexChanged(int index);
    void on_ckBox_SendByTime_checkStateChanged(const Qt::CheckState &arg1);
    void on_listWidget_customContextMenuRequested(const QPoint &pos);

private:
    void styleSheetUpdate();
    void loadCustomItem();
    void saveCustomItem();
    void saveConfig();
    void comPortSetting();
    void sendData(QString &content, SendModel model);
    void showSendData(const QByteArray &sendBuf);
    void labelInfoRefresh(const QString strInfo);
    void appendCheckData(QByteArray &sendBuf);
    void appendModbusCRC16(QByteArray &data);
    void appendAdd8(QByteArray &data);

protected:
    void closeEvent(QCloseEvent *event) override;
    Ui::MainWindow *ui;

/***************** 发送模块 *****************/
public slots:
    void on_lineEdit_SlaveAddress_textChanged(const QString &arg1);

    void on_btn_ModbusReadAdd_clicked();
    void on_btn_ModbusRedOn_clicked();
    void on_btn_ModbusRedOff_clicked();
    void on_btn_ModbusRedToggle_clicked();
    void on_btn_ModbusGreenOn_clicked();
    void on_btn_ModbusGreenOff_clicked();
    void on_btn_ModbusGreenToggle_clicked();
    void on_btn_ModbusBlueOn_clicked();
    void on_btn_ModbusBlueOff_clicked();
    void on_btn_ModbusBlueToggle_clicked();
    void on_btn_DHT11ReadTemp_clicked();
    void on_btn_DHT11ReadHumi_clicked();
    void on_btn_DHT11ReadTH_clicked();
    void on_btn_RTCSetTime_clicked();
    void on_btn_RTCGetTime_clicked();

private:
    ModbusTab *m_modbus = new ModbusTab(this);

};
#endif // MAINWINDOW_H
