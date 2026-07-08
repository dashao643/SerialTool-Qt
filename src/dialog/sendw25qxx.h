#ifndef SEND_W25QXX_H
#define SEND_W25QXX_H

#include "dataStructure.h"

#include <QDialog>
#include <QProgressBar>

namespace Ui {
class SendW25Qxx;
}

constexpr int FLASH_SECTOR_SIZE = 4096;

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
  Ui::SendW25Qxx *ui;

  int fileSize_ = 0;
  int pageCntInSector_ = 0;

  void labelDataRefresh(QString filePath);
  void pageCntRefresh(int flashSize, int pageSize);
  void sectorRangeRefresh(int basePage);
protected:

private slots:
  void on_btn_openFile_clicked();
  void on_btn_transmit_clicked();
  void on_spinBox_flashPageSize_valueChanged(int i);
  void on_spinBox_flashPageIdx_valueChanged(int i);
};

#endif // SENDFILEDIALOG_H
