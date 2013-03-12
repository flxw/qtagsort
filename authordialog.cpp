# include "authordialog.h"
# include "ui_authordialog.h"
# include "versioninfo.h"

AuthorDialog::AuthorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorDialog)
{
    ui->setupUi(this);
    this->ui->versionLabel->setText(QString(VERSION));
}

AuthorDialog::~AuthorDialog()
{
    delete ui;
}
