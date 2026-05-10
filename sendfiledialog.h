#ifndef SENDFILEDIALOG_H
#define SENDFILEDIALOG_H

#include "dataStructure.h"

#include <QDialog>

namespace Ui {
class SendFileDialog;
}


class SendFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendFileDialog(SendFile_t sendFileConfig, QWidget *parent = nullptr);
    ~SendFileDialog();
    SendFile_t getConfig() const;
    void setProgress(int value);

signals:
    void download(const SendFile_t &sendFile_);

private:
    SendFile_t config_;
    Ui::SendFileDialog *ui;

protected:

private slots:
    void on_btn_OpenFile_clicked();
    void on_btn_Download_clicked();
};

#endif // SENDFILEDIALOG_H
