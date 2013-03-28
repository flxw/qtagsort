# include "sametargetchoicedialog.h"
# include "ui_sametargetchoicedialog.h"

# include <QPushButton>

SameTargetChoiceDialog::SameTargetChoiceDialog(const QList<QStringList> &duplicateList, QWidget *parent) :
    QDialog(parent), ui(new Ui::SameTargetChoiceDialog), sourceFileList(duplicateList)
{
    ui->setupUi(this);

    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    QStringList destinations;

    /* the first index of a list inside duplicateList contains the expanded Pattern
     * at the same time, fill the chosenFiles list with -1s so that we know that no
     * files have been chosen yet */
    for (QList<QStringList>::iterator it = sourceFileList.begin();
         it != sourceFileList.end(); ++it) {
        destinations.append(it->takeAt(0));
        chosenFiles.append(-1);
    }

    this->destinationModel = new QStringListModel(destinations, this);
    this->sourceFileModel  = new QStringListModel(this);

    this->ui->destinationView->setModel(this->destinationModel);
    this->ui->sourceFileView->setModel(this->sourceFileModel);

    changeSourceDisplay(destinationModel->index(0));

    /* === connections ======================= */
    connect(this->ui->destinationView, SIGNAL(clicked(QModelIndex)), this, SLOT(changeSourceDisplay(QModelIndex)));
    connect(this->ui->sourceFileView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(markChosenAndMoveOn(QModelIndex)));
}

SameTargetChoiceDialog::~SameTargetChoiceDialog() {
    delete ui;
    delete destinationModel;
    delete sourceFileModel;
}

QStringList SameTargetChoiceDialog::getBadDuplicates() {
    QStringList retList;

    for (int i=0; i < sourceFileList.size(); ++i) {
        sourceFileList[i].takeAt(chosenFiles.at(i));
        retList.append(sourceFileList.at(i));
    }

    return retList;
}

/* =========================================== */
/*       Definition  of public slots           */
/* =========================================== */
void SameTargetChoiceDialog::changeSourceDisplay(const QModelIndex& newIndex) {
    if (currentIndex == newIndex.row()) return;

    this->currentIndex = newIndex.row();
    int chosenIndex = chosenFiles.at(currentIndex);
    if (chosenIndex >= 0) {
        this->ui->selectedDestinationLabel->setText(sourceFileList.at(currentIndex).at(chosenIndex));
    }

    this->sourceFileModel->setStringList(this->sourceFileList.at(currentIndex));
    this->checkForCompleteness();
}

void SameTargetChoiceDialog::markChosenAndMoveOn(const QModelIndex &dcIndex) {
    chosenFiles[currentIndex] = dcIndex.row();

    if (currentIndex+1 < sourceFileList.length()) {
        this->changeSourceDisplay(dcIndex.child(currentIndex+1, dcIndex.column()));
    } else {
        this->ui->selectedDestinationLabel->setText(sourceFileList.at(currentIndex).at(dcIndex.row()));
    }

    this->checkForCompleteness();
}

/* =========================================== */
/*       Definition  of private slots          */
/* =========================================== */
void SameTargetChoiceDialog::checkForCompleteness() {
    for (QList<int>::const_iterator it = this->chosenFiles.begin();
         it != this->chosenFiles.end(); ++it) {
        if (*it < 0) return;
    }

    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
