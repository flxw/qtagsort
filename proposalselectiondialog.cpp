# include "proposalselectiondialog.h"
# include "ui_proposalselectiondialog.h"

# include <QListView>
# include <QItemSelectionModel>
# include <QVariant>

ProposalSelectionDialog::ProposalSelectionDialog(QStringList tl, QStringList rl, QStringList al, QString filename, QWidget *parent) :
    QDialog(parent), ui(new Ui::ProposalSelectionDialog) {
    ui->setupUi(this);

    this->artistListModel  = new QStringListModel(al, this);
    this->titleListModel   = new QStringListModel(tl, this);
    this->releaseListModel = new QStringListModel(rl, this);

    this->ui->fileLabel->setText(tr("File: %1").arg(filename));
    this->artistListModel->setHeaderData(0, Qt::Horizontal, QVariant(tr("Artist")));
    this->titleListModel->setHeaderData(0, Qt::Horizontal, QVariant(tr("Title")));
    this->releaseListModel->setHeaderData(0, Qt::Horizontal, QVariant(tr("Album")));

    this->ui->titleView->setModel(titleListModel);
    this->ui->releaseView->setModel(releaseListModel);
    this->ui->artistView->setModel(artistListModel);
}

ProposalSelectionDialog::~ProposalSelectionDialog() {
    delete ui;
    delete artistListModel;
    delete titleListModel;
    delete releaseListModel;
}

QString ProposalSelectionDialog::getTitleSelection() {
    QList<QModelIndex> indexList = this->ui->titleView->selectionModel()->selectedRows();
    QString retVal;

    if (indexList.size() == 1) {
        retVal = titleListModel->data(indexList.at(0), Qt::DisplayRole).toString();
    }

    return retVal;
}

QString ProposalSelectionDialog::getReleaseSelection() {
    QList<QModelIndex> indexList = this->ui->releaseView->selectionModel()->selectedRows();
    QString retVal;

    if (indexList.size() == 1) {
        retVal = releaseListModel->data(indexList.at(0), Qt::DisplayRole).toString();
    }

    return retVal;
}

QString ProposalSelectionDialog::getArtistSelection() {
    QList<QModelIndex> indexList = this->ui->artistView->selectionModel()->selectedRows();
    QString retVal;

    if (indexList.size() == 1) {
        retVal = artistListModel->data(indexList.at(0), Qt::DisplayRole).toString();
    }

    return retVal;
}
