#include <QMessageBox>

#include "datafeeddialog.h"
#include "ui_datafeeddialog.h"
#include "walletmodel.h"

DataFeedDialog::DataFeedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataFeedDialog)
{
    ui->setupUi(this);
}

DataFeedDialog::~DataFeedDialog()
{
    delete ui;
}

void DataFeedDialog::setModel(WalletModel *model)
{
    this->model = model;
    ui->urlEdit->setText(model->getDataFeed());
}

bool DataFeedDialog::confirmAfterError(QString error)
{
    QMessageBox::StandardButton reply;

    QString sQuestion = tr("The initial download failed with the following error:\n%1\n\nDo you really want to set this URL as data feed?").arg(error);
    reply = QMessageBox::warning(this, tr("Data feed error confirmation"), sQuestion, QMessageBox::Yes | QMessageBox::No);
    return reply == QMessageBox::Yes;
}

void DataFeedDialog::accept()
{
    QString previousDataFeed = model->getDataFeed();
    model->setDataFeed(ui->urlEdit->text());
    try
    {
        model->updateFromDataFeed();
        QDialog::accept();
    }
    catch (std::exception& e)
    {
        model->setDataFeed(previousDataFeed);
        if (confirmAfterError(e.what()))
        {
            model->setDataFeed(ui->urlEdit->text());
            QDialog::accept();
        }
    }
}
