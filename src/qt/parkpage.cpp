#include "parkpage.h"
#include "ui_parkpage.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "parktablemodel.h"
#include "parkdialog.h"

#include <QSortFilterProxyModel>

ParkPage::ParkPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParkPage),
    model(0)
{
    ui->setupUi(this);
}

ParkPage::~ParkPage()
{
    delete ui;
}

void ParkPage::setModel(WalletModel *model)
{
    this->model = model;

    QTableView *view = ui->parkTableView;

    ParkTableModel *parkTableModel = model->getParkTableModel();

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(parkTableModel);
    proxyModel->setDynamicSortFilter(true);

    proxyModel->setSortRole(Qt::EditRole);

    view->setModel(proxyModel);

    view->setAlternatingRowColors(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setSortingEnabled(true);
    view->sortByColumn(ParkTableModel::Date, Qt::DescendingOrder);
    view->verticalHeader()->hide();

    view->horizontalHeader()->resizeSection(ParkTableModel::Status, 23);
    view->horizontalHeader()->resizeSection(ParkTableModel::Date, 120);
    view->horizontalHeader()->resizeSection(ParkTableModel::Amount, 100);
    view->horizontalHeader()->resizeSection(ParkTableModel::EstimatedTimeLeft, 100);
    view->horizontalHeader()->resizeSection(ParkTableModel::Premium, 100);
    view->horizontalHeader()->setResizeMode(ParkTableModel::UnparkAddress, QHeaderView::Stretch);
}

void ParkPage::on_park_clicked()
{
    ParkDialog dlg(this);
    dlg.setModel(model);
    dlg.exec();
}

