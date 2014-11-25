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
    setDataFeed(model->getDataFeed());
}

void DataFeedDialog::setDataFeed(const CDataFeed& dataFeed)
{
    ui->urlEdit->setText(QString::fromStdString(dataFeed.sURL));
    ui->signatureUrlEdit->setText(QString::fromStdString(dataFeed.sSignatureURL));
    ui->signatureAddressEdit->setText(QString::fromStdString(dataFeed.sSignatureAddress));
}

CDataFeed DataFeedDialog::getDataFeed() const
{
    CDataFeed dataFeed;
    dataFeed.sURL = ui->urlEdit->text().toUtf8().constData();
    dataFeed.sSignatureURL = ui->signatureUrlEdit->text().toUtf8().constData();
    dataFeed.sSignatureAddress = ui->signatureAddressEdit->text().toUtf8().constData();
    return dataFeed;
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
    CDataFeed previousDataFeed = model->getDataFeed();
    model->setDataFeed(getDataFeed());
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
            model->setDataFeed(getDataFeed());
            QDialog::accept();
        }
    }
}
