#include "authordialog.h"
#include "ui_authordialog.h"

AuthorDialog::AuthorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorDialog)
{
    ui->setupUi(this);
}

AuthorDialog::~AuthorDialog()
{
    delete ui;
}
