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
    this->ui->failureCountLabel->setText(QString::number(res.failed));
    this->ui->succesCountLabel->setText(QString::number(res.handled));

    this->show();
}
