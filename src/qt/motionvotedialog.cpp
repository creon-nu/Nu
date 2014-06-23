#include <QMessageBox>

#include "motionvotedialog.h"
#include "ui_motionvotedialog.h"

#include "walletmodel.h"

MotionVoteDialog::MotionVoteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MotionVoteDialog)
{
    ui->setupUi(this);
    setWindowTitle("Motion vote");
}

MotionVoteDialog::~MotionVoteDialog()
{
    delete ui;
}

void MotionVoteDialog::setModel(WalletModel *model)
{
    this->model = model;
    CVote vote = model->getVote();
    ui->motionHash->setText(QString::fromStdString(vote.hashMotion.GetHex()));
}

void MotionVoteDialog::error(const QString& message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void MotionVoteDialog::accept()
{
    CVote vote = model->getVote();
    QString motionHash(ui->motionHash->text());
    QRegExp rx("^[0-9A-Fa-f]*$");
    if (!rx.exactMatch(motionHash))
    {
        error(tr("The motion hash must only contain hexdecimal characters"));
        return;
    }
    vote.hashMotion.SetHex(motionHash.toAscii().data());
    model->setVote(vote);
    QDialog::accept();
}
