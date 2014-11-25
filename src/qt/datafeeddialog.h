#ifndef DATAFEEDDIALOG_H
#define DATAFEEDDIALOG_H

#include <QDialog>
#include "datafeed.h"

namespace Ui {
class DataFeedDialog;
}
class WalletModel;

class DataFeedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataFeedDialog(QWidget *parent = 0);
    ~DataFeedDialog();

    void setModel(WalletModel *model);
    void setDataFeed(const CDataFeed&);
    CDataFeed getDataFeed() const;

private:
    Ui::DataFeedDialog *ui;

    WalletModel *model;

    bool confirmAfterError(QString error);

private slots:
    void accept();
};

#endif // DATAFEEDDIALOG_H
