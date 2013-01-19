#include "resultdialog.h"
#include "ui_resultdialog.h"

ResultDialog::ResultDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResultDialog)
{
    ui->setupUi(this);
}

ResultDialog::~ResultDialog()
{
    delete ui;
}


void ResultDialog::prepareShow(FileHandler::HandleReport res) {
    const unsigned int total = res.failed + res.handled;

    this->ui->failureCountLabel->setText(tr("Failed %1").arg(res.failed));
    this->ui->succesCountLabel->setText(tr("Successful: %1").arg(res.handled));

    this->ui->failureBar->setMaximum(total);
    this->ui->successBar->setMaximum(total);

    this->ui->failureBar->setValue(res.failed);
    this->ui->successBar->setValue(res.handled);

    this->show();
}
