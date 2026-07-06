#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

#include "dataStructure.h"

#include <QWidget>
#include <QPushButton>

constexpr int CUSTOM_ITEM_HEIGHT = 70;

namespace Ui {
class CustomItem;
}

class CustomItem : public QWidget
{
  Q_OBJECT

public:
  explicit CustomItem(QWidget *parent = nullptr);
  ~CustomItem();

  void setRemark(const QString remark);
  void setContent(const QString content);
  void setModel(const SendModel model);

  QString getRemark();
  QString getContent();
  SendModel getModel();

  QPushButton* getSendBtn();

private:
  Ui::CustomItem *ui;

private slots:
  void on_btn_Send_clicked();

signals:
  void sendDataRequest(QString content, SendModel sendmodel);
};

#endif // CUSTOMITEM_H
