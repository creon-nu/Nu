#include <QMessageBox>
#include <QThread>

#include "datafeeddialog.h"
#include "ui_datafeeddialog.h"
#include "walletmodel.h"

DataFeedDialog::DataFeedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataFeedDialog),
    waitDialog(this)
{
    ui->setupUi(this);

    waitDialog.setWindowTitle(tr("Data feed update"));
    waitDialog.setText(tr("Fetching feed, please wait..."));
    waitDialog.setStandardButtons(QMessageBox::NoButton);
    waitDialog.setIcon(QMessageBox::Information);
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
    const std::vector<std::string>& vParts = dataFeed.vParts;
    ui->custodiansCheckBox->setChecked(std::find(vParts.begin(), vParts.end(), "custodians") != vParts.end());
    ui->parkRatesCheckBox->setChecked(std::find(vParts.begin(), vParts.end(), "parkrates") != vParts.end());
    ui->motionsCheckBox->setChecked(std::find(vParts.begin(), vParts.end(), "motions") != vParts.end());
    ui->feesCheckBox->setChecked(std::find(vParts.begin(), vParts.end(), "fees") != vParts.end());
}

CDataFeed DataFeedDialog::getDataFeed() const
{
    CDataFeed dataFeed;
    dataFeed.sURL = ui->urlEdit->text().toUtf8().constData();
    dataFeed.sSignatureURL = ui->signatureUrlEdit->text().toUtf8().constData();
    dataFeed.sSignatureAddress = ui->signatureAddressEdit->text().toUtf8().constData();
    std::vector<std::string> vParts;
    if (ui->custodiansCheckBox->isChecked())
        vParts.push_back("custodians");
    if (ui->parkRatesCheckBox->isChecked())
        vParts.push_back("parkrates");
    if (ui->motionsCheckBox->isChecked())
        vParts.push_back("motions");
    if (ui->feesCheckBox->isChecked())
        vParts.push_back("fees");
    dataFeed.vParts = vParts;
    return dataFeed;
}

bool DataFeedDialog::confirmAfterError(QString error)
{
    QMessageBox::StandardButton reply;

    QString sQuestion = tr("The initial download failed with the following error:\n%1\n\nDo you really want to set this URL as data feed?").arg(error);
    reply = QMessageBox::warning(this, tr("Data feed error confirmation"), sQuestion, QMessageBox::Yes | QMessageBox::No);
    return reply == QMessageBox::Yes;
}

class UpdateDataFeedThread : public QThread
{
    Q_OBJECT

    WalletModel *model;

public:
    UpdateDataFeedThread(QObject* parent, WalletModel *model) :
        QThread(parent),
        model(model)
    {
    }

protected:
    void run()
    {
        try
        {
            model->updateFromDataFeed();
            emit success();
        }
        catch (std::exception& e)
        {
            emit failure(QString::fromStdString(e.what()));
        }
    }

signals:
    void success();
    void failure(QString error);
};

void DataFeedDialog::close()
{
    waitDialog.setVisible(false);
    QDialog::accept();
}

void DataFeedDialog::revertAndAskForConfirmation(QString error)
{
    waitDialog.setVisible(false);
    model->setDataFeed(previousDataFeed);
    if (confirmAfterError(error))
    {
        model->setDataFeed(getDataFeed());
        close();
    }
}

void DataFeedDialog::error(const QString& message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void DataFeedDialog::accept()
{
    if (ui->signatureUrlEdit->text().isEmpty() != ui->signatureAddressEdit->text().isEmpty())
    {
        error(tr("The signature URL and address fields must be both filled or both empty."));
        return;
    }

    waitDialog.setVisible(true);

    previousDataFeed = model->getDataFeed();
    model->setDataFeed(getDataFeed());

    UpdateDataFeedThread *thread = new UpdateDataFeedThread(this, model);
    connect(thread, SIGNAL(success()), this, SLOT(close()));
    connect(thread, SIGNAL(failure(QString)), this, SLOT(revertAndAskForConfirmation(QString)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

#include "datafeeddialog.moc"

