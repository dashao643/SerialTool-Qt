#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    qint8 defaultBaudRateIndex = 0;
    // QTimer laberInfoTimer;                  // 信息显示清除计时器

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void uiInit();
    void btnInit();

private slots:
    void cbBoxPortNumRefresh();
    void btnOpenClose();
    void on_actPortSetting_triggered();
    void on_actClear_triggered();
    void on_actFont_triggered();

private:
    void labelInfoRefresh(QString strInfo);
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
