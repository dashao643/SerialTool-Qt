#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "appconfig.h"
#include "serialmanager.h"
#include "networkmanager.h"
#include "dialog/sendfiledialog.h"
#include "tab_page/list_item/customitem.h"

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

// const QString APP_VERSION = "1.2.1";

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  QTimer *infoTimer_ = nullptr;
  QTimer *rxTimer_ = nullptr;
  QTimer *sendTimer_ = nullptr;

  QByteArray receiveBuffer_;                             // 接收缓冲区
  QString fileSavePath_;
  SendFile_t sendFile_;

  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void cbBoxInit();
  void dataInit();
  void uiInit();
  void slotsInit();

private:
  AppConfig *appConfig_;
  SerialManager *serialManager_;
  NetworkManager *networkManager_;
  bool isConnected_ = false;
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
  void do_netWorkStateUpdate(QAbstractSocket::SocketState state);
  void do_readyRead(const QByteArray &byteArray);
  void do_showReceivedData();
  void do_calSelCharCnt();
  CustomItem* do_addItemToList(int row = 0, bool isInsert = false);
  void do_fileDownload(const SendFile_t &sendFile, SendFileDialog* dialog);

  void on_btn_OpenClose_clicked();
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
