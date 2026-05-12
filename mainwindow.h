#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "appconfig.h"
#include "serialmanager.h"
#include "networkmanager.h"
#include "sendfiledialog.h"

#include <QMainWindow>
#include <QTimer>

constexpr int DEFAULT_WINDOW_WIDTH = 1024;
constexpr int DEFAULT_WINDOW_HEIGHT = 800;
constexpr int LABEL_INFO_SHOW_MS = 1000;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QTimer *infoTimer_ = nullptr;
    QTimer *rxTimer_;
    QTimer *sendTimer_;

    QByteArray receiveBuffer_;                             // 接收缓冲区
    QString fileSavePath_;
    SendFile_t sendFile_;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void dataInit();
    void uiInit();
    void slotsInit();

private:
    AppConfig *appConfig_;
    SerialManager *serialManager_;
    NetworkManager *networkManager_;
    bool isFileDownload = false;
    QByteArray fileReceiveBuffer_;
    Ui::MainWindow *ui;

    void loadPageConfig();
    void sendData(QString content, SendModel model = HEX);
    void showSendData(const QByteArray &sendBuf);
    void labelInfoRefresh(const QString strInfo);
    void appendCheckData(QByteArray &sendBuf);
    void appendModbusCRC16(QByteArray &data);
    void appendAdd8(QByteArray &data);
    bool waitAck(const QString &targetAck, int timeoutMs);

private slots:
    void do_UiUpdate(bool isSerialOpen);
    void do_serialReadyRead();
    void do_showReceivedData();
    void do_calSelCharCnt();
    void do_addItemToList(int row = 0, bool isInsert = false);
    void do_fileDownload(const SendFile_t &sendFile, SendFileDialog* dialog);

    void on_btn_Send_clicked();
    void on_rdBtn_ShowASCII_clicked();
    void on_actPortSetting_triggered();
    void on_actSave_triggered();
    void on_actFont_triggered();
    void on_actAddTab_triggered();
    void on_actDelTab_triggered();
    void on_actBackgroundSetting_triggered();

    void on_tabWidget_tabBarDoubleClicked(int index);
    void on_tabWidget_customContextMenuRequested(const QPoint &pos);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_toolBar_customContextMenuRequested(const QPoint &pos);
    void on_actTimeSyn_triggered();
};
#endif // MAINWINDOW_H
