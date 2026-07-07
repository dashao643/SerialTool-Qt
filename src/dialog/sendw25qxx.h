#ifndef SEND_W25QXX_H
#define SEND_W25QXX_H

#include "dataStructure.h"

#include <QDialog>
#include <QProgressBar>

namespace Ui {
class SendW25Qxx;
}

class SendW25Qxx : public QDialog
{
  Q_OBJECT

public:
  explicit SendW25Qxx(const SendW25Qxx_t &config, QWidget *parent = nullptr);
  ~SendW25Qxx();
  SendW25Qxx_t getConfig() const;
  QProgressBar* getProgress(void);

signals:
  void tranmit(const SendW25Qxx_t &sendFile_);

private:
  // SendW25Qxx_t config_;
  Ui::SendW25Qxx *ui;

protected:

private slots:
  void on_btn_openFile_clicked();
  void on_btn_transmit_clicked();
};

#endif // SENDFILEDIALOG_H
