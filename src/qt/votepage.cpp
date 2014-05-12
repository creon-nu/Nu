#include "votepage.h"
#include "ui_votepage.h"
#include "walletmodel.h"

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
}

void VotePage::on_parkRateVote_clicked()
{
}

void VotePage::on_motionVote_clicked()
{
}
