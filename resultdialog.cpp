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


void ResultDialog::prepareShow(const int &good, const int &bad) {
    this->ui->failureCountLabel->setText(QString::number(bad));
    this->ui->succesCountLabel->setText(QString::number(good));

    this->show();
}
