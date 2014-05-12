#ifndef VOTEPAGE_H
#define VOTEPAGE_H

#include <QWidget>

namespace Ui {
class VotePage;
}
class WalletModel;

class VotePage : public QWidget
{
    Q_OBJECT

public:
    explicit VotePage(QWidget *parent = 0);
    ~VotePage();

    void setModel(WalletModel *model);

private:
    Ui::VotePage *ui;
    WalletModel *model;

private slots:
    void on_custodianVote_clicked();
    void on_parkRateVote_clicked();
    void on_motionVote_clicked();
};

#endif // VOTEPAGE_H
