#ifndef TABPAGE_H
#define TABPAGE_H

#include <QWidget>
#include <QListWidget>

namespace Ui {
class TabPage;
}

class TabPage : public QWidget
{
    Q_OBJECT

public:
    explicit TabPage(QWidget *parent = nullptr);
    ~TabPage();

    QListWidget* getListWidget();

private slots:
    void on_listWidget_customContextMenuRequested(const QPoint &pos);
    void on_actDelItem_triggered();
    void on_actInsert_triggered();

signals:
    void showMessage(const QString& text);
    void insertItem(int index);

private:
    Ui::TabPage *ui;
};

#endif // TABPAGE_H
