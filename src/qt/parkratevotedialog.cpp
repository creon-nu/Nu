#include <QMessageBox>

#include "parkratevotedialog.h"
#include "ui_parkratevotedialog.h"

#include "walletmodel.h"
#include "vote.h"
#include "optionsmodel.h"
#include "bitcoinunits.h"
#include "guiutil.h"

using namespace std;

ParkRateVoteDialog::ParkRateVoteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParkRateVoteDialog)
{
    ui->setupUi(this);
    setWindowTitle("Park rate vote");
}

ParkRateVoteDialog::~ParkRateVoteDialog()
{
    delete ui;
}

class QCompactDurationTableWidgetItem : public QTableWidgetItem
{
public:
    QCompactDurationTableWidgetItem(int compactDuration) :
        QTableWidgetItem(compactDuration),
        compactDuration(compactDuration)
    {
    }

    QVariant data(int role = Qt::UserRole + 1) const
    {
        switch (role)
        {
            case Qt::EditRole:
                return compactDuration;
            case Qt::DisplayRole:
                return GUIUtil::blocksToTime((qint64)1 << compactDuration);
            default:
                return QTableWidgetItem::data(role);
        }
    }

    void setData(int role, const QVariant& value)
    {
        if (role == Qt::EditRole)
            compactDuration = value.toInt();
        else
            QTableWidgetItem::setData(role, value);
    }

    int getCompactDuration() const
    {
        return compactDuration;
    }

private:
    int compactDuration;
};

void ParkRateVoteDialog::setModel(WalletModel *model)
{
    this->model = model;
    CVote vote = model->getVote();

    CParkRateVote parkRateVote;
    for (int i = 0; i < vote.vParkRateVote.size(); i++)
        if (vote.vParkRateVote[i].cUnit == 'B')
        {
            parkRateVote = vote.vParkRateVote[i];
            break;
        }

    const vector<CParkRate>& vParkRate = parkRateVote.vParkRate;

    ui->table->setRowCount(vParkRate.size());
    for (int i = 0; i < vParkRate.size(); i++)
    {
        const CParkRate& parkRate = vParkRate[i];

        setDuration(i, parkRate.nCompactDuration);
        setAnnualRatePercentage(i, GUIUtil::annualInterestRatePercentage(parkRate.nRate, parkRate.GetDuration()));
    }

    ui->table->setVisible(false);
    ui->table->resizeColumnsToContents();
    ui->table->setVisible(true);
}

double ParkRateVoteDialog::getAnnualRatePercentage(int row)
{
    QTableWidgetItem *rateItem = ui->table->item(row, 1);
    if (rateItem)
        return rateItem->data(Qt::DisplayRole).toDouble();
    else
        return 0;
}

qint64 ParkRateVoteDialog::getRate(int row)
{
    return GUIUtil::annualInterestRatePercentageToRate(getAnnualRatePercentage(row), getDuration(row));
}

qint64 ParkRateVoteDialog::getDuration(int row)
{
    return (qint64)1 << getCompactDuration(row);
}

int ParkRateVoteDialog::getCompactDuration(int row)
{
    QCompactDurationTableWidgetItem *durationItem = (QCompactDurationTableWidgetItem*)ui->table->item(row, 0);
    if (!durationItem)
        return 0;
    return durationItem->getCompactDuration();
}

void ParkRateVoteDialog::on_table_cellChanged(int row, int column)
{
    if (column == 1)
    {
        // Round to nearest possible annual interest rate
        qint64 rate = getRate(row);
        double interestRatePercentage = GUIUtil::annualInterestRatePercentage(rate, getDuration(row));
        QTableWidgetItem *rateItem = ui->table->item(row, column);
        rateItem->setData(Qt::DisplayRole, interestRatePercentage);
    }
}

void ParkRateVoteDialog::setDuration(int row, qint64 duration)
{
    QTableWidgetItem *durationItem = new QCompactDurationTableWidgetItem(duration);
    durationItem->setData(Qt::TextAlignmentRole, QVariant(Qt::AlignRight | Qt::AlignVCenter));
    durationItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->table->setItem(row, 0, durationItem);
}

void ParkRateVoteDialog::setAnnualRatePercentage(int row, double rate)
{
    QTableWidgetItem *rateItem = new QTableWidgetItem();
    rateItem->setData(Qt::DisplayRole, QVariant(rate));
    rateItem->setData(Qt::TextAlignmentRole, QVariant(Qt::AlignRight | Qt::AlignVCenter));
    ui->table->setItem(row, 1, rateItem);
}

void ParkRateVoteDialog::on_addShorter_clicked()
{
    int duration = 17;
    double rate = 1.0;
    if (ui->table->rowCount() > 0)
    {
        duration = getCompactDuration(0) - 1;
        rate = getAnnualRatePercentage(0);
    }
    if (duration < 0)
        return;

    ui->table->insertRow(0);

    setDuration(0, duration);
    setAnnualRatePercentage(0, rate);
}

void ParkRateVoteDialog::on_addLonger_clicked()
{
    int duration = 17;
    double rate = 1.0;
    int rowCount = ui->table->rowCount();

    if (rowCount > 0)
    {
        duration = getCompactDuration(rowCount - 1) + 1;
        rate = getAnnualRatePercentage(rowCount - 1);
    }
    if (duration <= 0 || duration >= 30)
        return;

    ui->table->setRowCount(rowCount + 1);

    setDuration(rowCount, duration);
    setAnnualRatePercentage(rowCount, rate);
}

void ParkRateVoteDialog::on_remove_clicked()
{
    QItemSelection selection(ui->table->selectionModel()->selection());

    QList<int> rows;
    foreach(const QModelIndex& index, selection.indexes())
        rows.append(index.row());

    qSort(rows);

    int prev = -1;
    for (int i = rows.count() - 1; i >= 0; i -= 1)
    {
        int current = rows[i];
        if (current != prev)
        {
            ui->table->removeRow(current);
            prev = current;
        }
    }
}

void ParkRateVoteDialog::error(const QString& message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void ParkRateVoteDialog::accept()
{
    QTableWidget *table = ui->table;
    int rows = table->rowCount();
    CParkRateVote parkRateVote;
    parkRateVote.cUnit = 'B';
    vector<CParkRate>& vParkRate = parkRateVote.vParkRate;

    for (int i = 0; i < rows; i++)
    {
        CParkRate parkRate;
        parkRate.nCompactDuration = getCompactDuration(i);
        parkRate.nRate = getRate(i);
        vParkRate.push_back(parkRate);
    }

    CVote vote = model->getVote();
    vote.vParkRateVote.clear();
    vote.vParkRateVote.push_back(parkRateVote);
    model->setVote(vote);
    QDialog::accept();
}
