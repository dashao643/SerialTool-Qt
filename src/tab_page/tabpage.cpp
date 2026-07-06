#include "tabpage.h"
#include "ui_tabpage.h"

#include <QMenu>
#include <QMessageBox>

TabPage::TabPage(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::TabPage)
{
  ui->setupUi(this);
}

TabPage::~TabPage()
{
  delete ui;
}

QListWidget *TabPage::getListWidget()
{
  return ui->listWidget;
}

void TabPage::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
  // 获取点击位置
  QPoint globalPos = ui->listWidget->mapToGlobal(pos);

  // 创建菜单
  QMenu menu;
  menu.addAction(ui->actInsert);
  menu.addSeparator();
  menu.addAction(ui->actDelItem);

  // 弹出菜单
  menu.exec(globalPos);
}

void TabPage::on_actDelItem_triggered()
{
  int row = ui->listWidget->currentRow();
  if(row == -1){
    QMessageBox::information(this,"提示","未选中");
    return;
  }
  QListWidgetItem* item = ui->listWidget->takeItem(row);
  delete item;
  QMessageBox::information(this,"提示",QString("第%1项已删除").arg(row + 1));
}

void TabPage::on_actInsert_triggered()
{
  int index = ui->listWidget->currentRow();
  emit insertItem(index);
}

