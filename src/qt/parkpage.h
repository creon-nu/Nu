#ifndef PARKPAGE_H
#define PARKPAGE_H

#include <QDialog>

namespace Ui {
class ParkPage;
}
class WalletModel;

class ParkPage : public QDialog
{
    Q_OBJECT

public:
    explicit ParkPage(QWidget *parent = 0);
    ~ParkPage();

    void setModel(WalletModel *model);

private:
    Ui::ParkPage *ui;
    WalletModel *model;

private slots:
    void on_park_clicked();
};

#endif // PARKPAGE_H
