# include "sourcefileeditdialog.h"
# include "ui_sourcefileeditdialog.h"
# include "mainwindow.h"

# include <QFileInfo>
# include <QUrl>
# include <QDirIterator>
# include <QFileDialog>

SourceFileEditDialog::SourceFileEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SourceFileEditDialog)
{
    ui->setupUi(this);

    /* initialize attributes */
    this->striLiMod = new QStringListModel(this->striLi);
    this->ui->listView->setModel(this->striLiMod);

    connect(this->ui->removeButton, SIGNAL(clicked()), this, SLOT(removeListEntry()));
    connect(this->ui->addButton, SIGNAL(clicked()), this, SLOT(addListEntry()));
}

SourceFileEditDialog::~SourceFileEditDialog()
{
    delete ui;
}

/* =========================================== */
/*       Definition of public methods          */
/* =========================================== */
void SourceFileEditDialog::dragEnterEvent(QDragEnterEvent *ev) {
    /* TODO: check MIME type here */
    ev->acceptProposedAction();
}

void SourceFileEditDialog::dropEvent(QDropEvent *event) {
    const QMimeData *mD = event->mimeData();

    /* check if we can get the location of the files dragged into the window */
    if (mD->hasUrls()) {
        QList<QUrl> urlList = mD->urls();

        QString tempFilename;

        /* check if local file or folder */
        for (int i=0; i < urlList.size(); ++i) {
            tempFilename = urlList.at(i).toLocalFile();

            /* check wether the path leads to a file */
            if (tempFilename.contains(".")) {
                ++this->entriesRead;
                this->checkAndAddFile(tempFilename);
            } else {
                /* the path leads to a directory, walk it! */
                QDirIterator dirIt(tempFilename, QDirIterator::Subdirectories);

                while ( dirIt.hasNext() ) {
                    dirIt.next();

                    ++this->entriesRead;
                    if ( dirIt.fileInfo().isFile() ) {
                        this->checkAndAddFile(dirIt.filePath());
                    }
                }
            }
        }
    }
}

void SourceFileEditDialog::setStringList(const QStringList &sl) {
    this->striLi = sl;
    this->striLiMod->setStringList(sl);
}

const QStringList& SourceFileEditDialog::getStringList(void) const {
    return this->striLi;
}

/* =========================================== */
/*       Definition of private methods         */
/* =========================================== */
void SourceFileEditDialog::checkAndAddFile(const QString &fpath) {
    if (fpath.contains(((MainWindow*)(this->parent()))->supportedFiletypes)) {
        QFileInfo fi(fpath);
        if (fi.isReadable()) {
            this->striLi.append(fpath);
            this->striLiMod->setStringList(this->striLi);
        }
    }
}

/* =========================================== */
/*       Definition of private slots           */
/* =========================================== */
void SourceFileEditDialog::addListEntry() {
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select files that shall be added to the sorting list"));

    for (QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
        this->checkAndAddFile(*it);
    }
}

void SourceFileEditDialog::removeListEntry() {
    QModelIndexList mi = this->ui->listView->selectionModel()->selectedRows();

    for (int i=0; i<mi.length(); ++i) {
        this->striLiMod->removeRow(mi.at(i).row());
    }
}
