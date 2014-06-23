#include <QTimer>

#include "votepage.h"
#include "ui_votepage.h"
#include "walletmodel.h"
#include "custodianvotedialog.h"
#include "parkratevotedialog.h"
#include "motionvotedialog.h"
#include "guiconstants.h"
#include "optionsmodel.h"
#include "bitcoinunits.h"
#include "guiutil.h"

VotePage::VotePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VotePage),
    model(NULL),
    lastBestBlock(NULL)
{
    ui->setupUi(this);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(MODEL_UPDATE_DELAY);
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

void VotePage::update()
{
    if (!model)
        return;

    {
        LOCK(cs_main);
        if (pindexBest == lastBestBlock)
            return;

        lastBestBlock = pindexBest;

        fillCustodianTable();
        fillParkRateTable();
    }
}

void VotePage::fillCustodianTable()
{
    QTableWidget* table = ui->custodianTable;
    table->setRowCount(0);
    int row = 0;
    for (CBlockIndex* pindex = lastBestBlock; pindex; pindex = pindex->pprev)
    {
        BOOST_FOREACH(const CCustodianVote& custodianVote, pindex->vElectedCustodian)
        {
            table->setRowCount(row + 1);

            QTableWidgetItem *addressItem = new QTableWidgetItem();
            addressItem->setData(Qt::DisplayRole, QString::fromStdString(custodianVote.GetAddress().ToString()));
            table->setItem(row, 0, addressItem);

            QTableWidgetItem *amountItem = new QTableWidgetItem();
            amountItem->setData(Qt::DisplayRole, BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), custodianVote.nAmount));
            amountItem->setData(Qt::TextAlignmentRole, QVariant(Qt::AlignRight | Qt::AlignVCenter));
            table->setItem(row, 1, amountItem);

            QTableWidgetItem *dateItem = new QTableWidgetItem();
            dateItem->setData(Qt::DisplayRole, GUIUtil::dateTimeStr(pindex->nTime));
            table->setItem(row, 2, dateItem);
        }
    }
    table->setVisible(false);
    table->resizeColumnsToContents();
    table->setVisible(true);
}

void VotePage::fillParkRateTable()
{
    QTableWidget* table = ui->parkRateTable;
    std::vector<CParkRate> parkRates;

    BOOST_FOREACH(const CParkRateVote& parkRateResult, lastBestBlock->vParkRateResult)
    {
        if (parkRateResult.cUnit == 'B')
        {
            parkRates = parkRateResult.vParkRate;
            break;
        }
    }

    table->setRowCount(parkRates.size());
    for (int i = 0; i < parkRates.size(); i++)
    {
        const CParkRate& parkRate = parkRates[i];

        QString durationString = GUIUtil::blocksToTime(parkRate.GetDuration());
        QTableWidgetItem *durationItem = new QTableWidgetItem(durationString);
        durationItem->setData(Qt::TextAlignmentRole, QVariant(Qt::AlignRight | Qt::AlignVCenter));
        table->setItem(i, 0, durationItem);

        double interestRate = GUIUtil::annualInterestRatePercentage(parkRate.nRate, parkRate.GetDuration());
        QString rateString = QString("%L1%").arg(interestRate, 0, 'f', 3);
        QTableWidgetItem *rateItem = new QTableWidgetItem(rateString);
        rateItem->setData(Qt::TextAlignmentRole, QVariant(Qt::AlignRight | Qt::AlignVCenter));
        table->setItem(i, 1, rateItem);
    }
    table->setVisible(false);
    table->resizeColumnsToContents();
    table->setVisible(true);
}
