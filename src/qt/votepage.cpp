#include "votepage.h"
#include "ui_votepage.h"
#include "walletmodel.h"
#include "custodianvotedialog.h"
#include "parkratevotedialog.h"
#include "motionvotedialog.h"

VotePage::VotePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VotePage)
{
    ui->setupUi(this);
}

VotePage::~VotePage()
{
    delete ui;
}

void VotePage::setModel(WalletModel *model)
{
    this->model = model;
}

void VotePage::on_custodianVote_clicked()
{
    CustodianVoteDialog dlg(this);
    dlg.setModel(model);
    dlg.exec();
}

void VotePage::on_parkRateVote_clicked()
{
    ParkRateVoteDialog dlg(this);
    dlg.setModel(model);
    dlg.exec();
}

void VotePage::on_motionVote_clicked()
{
    MotionVoteDialog dlg(this);
    dlg.setModel(model);
    dlg.exec();
}
