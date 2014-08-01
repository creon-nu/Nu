#ifndef PARKRATEVOTEDIALOG_H
#define PARKRATEVOTEDIALOG_H

#include <QDialog>

namespace Ui {
class ParkRateVoteDialog;
}
class WalletModel;

class ParkRateVoteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParkRateVoteDialog(QWidget *parent = 0);
    ~ParkRateVoteDialog();

    void setModel(WalletModel *model);

    double getAnnualRatePercentage(int row);
    qint64 getRate(int row);
    qint64 getDuration(int row);
    int getCompactDuration(int row);

    void setDuration(int row, qint64 duration);
    void setAnnualRatePercentage(int row, double rate);

private:
    Ui::ParkRateVoteDialog *ui;

    WalletModel *model;

    void error(const QString& message);

private slots:
    void on_addShorter_clicked();
    void on_addLonger_clicked();
    void on_remove_clicked();
    void on_table_cellChanged(int row, int column);
    void accept();
};

#endif // PARKRATEVOTEDIALOG_H
