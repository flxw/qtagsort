# include "mainwindow.h"
# include "ui_mainwindow.h"
# include "versioninfo.h"

# include <QFileDialog>
# include <QUrl>
# include <QMimeData>
# include <QDirIterator>
# include <QMessageBox>
# include <QTableView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),  supportedFiletypes(QRegExp(".(mp3|wma|acc|ogg)")), ui(new Ui::MainWindow) {
    /* initialize the user interface object */
    ui->setupUi(this);

    /* set some default values =============== */
    this->patternValidator = new PatternValidator(this);
    this->authorDialog     = new AuthorDialog(this);
    this->resultDialog     = new ResultDialog(this);
    this->musicDataModel   = new MusicDataModel(this);
    this->fileHandler      = new FileHandler(this->musicDataModel, this);

    this->ui->patternEdit->setValidator(this->patternValidator);

    this->ui->tableView->setModel(this->musicDataModel);
    this->ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    this->setWindowTitle(QString(PROGNAME));

    /* signal handler setup ================== */
    /* UI -- button functionality for program operation parameters */
    connect(this->ui->patternEdit, SIGNAL(textChanged(QString)), this, SLOT(reactOnPatternChange(QString)));
    connect(this->ui->sTargetButton, SIGNAL(clicked()), this, SLOT(setDestPath()));
    /* UI -- functionality for tableview */
    connect(this->ui->addEntryButton, SIGNAL(clicked()), this, SLOT(addToDB()));
    connect(this->ui->delEntryButton, SIGNAL(clicked()), this, SLOT(deleteDBEntry()));
    connect(this->ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(showFileLocation(QModelIndex)));
    connect(this->ui->autotagSelectedButton, SIGNAL(clicked()), this, SLOT(dispatchAutotag()));
    /* UI -- action connections */
    connect(this->ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(this->ui->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(showAboutQt()));
    connect(this->ui->actionAuthor, SIGNAL(triggered()), this->authorDialog, SLOT(show()));
    connect(this->ui->beginSortButton, SIGNAL(clicked()), this->fileHandler, SLOT(startSortAction()));

    /* operation aftermath information display */
    connect(this->fileHandler, SIGNAL(handleProgressPerc(int)), this->ui->progressBar, SLOT(setValue(int)));
    connect(this->fileHandler, SIGNAL(finished(FileHandler::HandleReport)), this->resultDialog, SLOT(prepareShow(FileHandler::HandleReport)));
    connect(this->resultDialog, SIGNAL(finished(int)), this, SLOT(cleanup()));
}

MainWindow::~MainWindow() {
    delete ui;
    delete fileHandler;
    delete patternValidator;
    delete authorDialog;
    delete resultDialog;
    delete musicDataModel;
}


/* =========================================== */
/*       Definition  of public functions       */
/* =========================================== */
QString MainWindow::expandExamplePattern() {
    QString pattern = this->ui->patternEdit->text();

    pattern.replace(MusicDataModel::PH_ARTIST,  "Artist");
    pattern.replace(MusicDataModel::PH_ALBUM,   "Album");
    pattern.replace(MusicDataModel::PH_TITLE,   "Track");
    pattern.replace(MusicDataModel::PH_TRACKNO, "Track Number");
    pattern.replace(MusicDataModel::PH_YEAR,    "Year");

    return pattern;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev) {
    /* TODO: check MIME type here */
    ev->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mD = event->mimeData();

    unsigned int read      = 0;
    unsigned int processed = 0;

    /* check if we can get the location of the files dragged into the window */
    if (mD->hasUrls()) {
        QList<QUrl> urlList = mD->urls();

        QString tempFilename;

        /* check if local file or folder */
        for (int i=0; i < urlList.size(); ++i) {
            tempFilename = urlList.at(i).toLocalFile();
            QFileInfo fi(tempFilename);

            /* check wether the path leads to a file */
            if (fi.isFile() && tempFilename.contains(this->supportedFiletypes)) {
                ++read;
                this->processFile(tempFilename, processed);
            } else if (fi.isDir()){
                /* the path leads to a directory, walk it! */
                QDirIterator dirIt(tempFilename, QDirIterator::Subdirectories);

                while (dirIt.hasNext()) {
                    dirIt.next();

                    ++read;
                    if ( dirIt.fileInfo().isFile() ) {
                        this->processFile(dirIt.filePath(), processed);
                    }
                }
            }
        }
        this->ui->statusBar->showMessage(tr("%1 entries read, %2 entries processed").arg(read).arg(processed));
        this->checkIfReadyForOp();

    }
}

/* =========================================== */
/*       Definition  of public slots           */
/* =========================================== */
void MainWindow::reactOnPatternChange(QString p) {
    if ( this->patternValidator->isValid(p) ) {
        this->musicDataModel->setPattern(p);
        this->ui->parsedPatternDispLabel->setText(this->expandExamplePattern());
    } else {
        this->musicDataModel->setPattern(QString());
        this->ui->parsedPatternDispLabel->setText("");
    }

    this->checkIfReadyForOp();
}

void MainWindow::setDestPath(void) {
    QString dest = QFileDialog::getExistingDirectory(this,\
                                                    tr("Select the directory that shall contain the sorted files"),\
                                                    QDir::homePath());
    QFileInfo destInfo(dest);

    if (destInfo.isWritable()) {
        this->musicDataModel->setTargetDir(dest);
        this->ui->targetDispLabel->setText(dest);
        this->checkIfReadyForOp();
    } else {
        this->ui->statusBar->showMessage(tr("Error: destination directory not writable!"), 5000);
    }
}

void MainWindow::showAboutQt() {
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::showFileLocation(const QModelIndex &mdi) {
    if (this->ui->tableView->selectionModel()->selectedRows().size() == 1) {
        this->ui->fileLocLabel->setText(this->musicDataModel->getFileLocation(mdi));
    } else {
        this->ui->fileLocLabel->setText(QString());
    }
}

void MainWindow::cleanup() {
    this->musicDataModel->clearData();

    this->ui->progressBar->setValue(0);

    this->checkIfReadyForOp();
}

/* > on the tag tab */
void MainWindow::addToDB() {
    // select either files or folders with dialog and
    // add like added with DragEvent
    QFileDialog fd(this, tr("Select the files that you want to sort by tags"));
    fd.setFileMode(QFileDialog::ExistingFiles);

    if (fd.exec() == QFileDialog::Accepted) {
        /* TODO: rewrite with QDirIterator... */
        QStringList strl = fd.selectedFiles();
        QStringList::iterator strlIt = strl.begin();

        unsigned int read      = 0;
        unsigned int processed = 0;

        for (; strlIt != strl.end(); ++strlIt) {
            QFileInfo fi(*strlIt);

            /* check wether the path leads to a file */
            if (fi.isFile() && strlIt->contains(this->supportedFiletypes)) {
                ++read;
                this->processFile(*strlIt, processed);
            } else if (fi.isDir()){
                /* the path leads to a directory, walk it! */
                QDirIterator dirIt(*strlIt, QDirIterator::Subdirectories);

                while (dirIt.hasNext()) {
                    dirIt.next();

                    ++read;
                    if (dirIt.fileInfo().isFile()) {
                        this->processFile(dirIt.filePath(), processed);
                    }
                }
            }
        }
        this->ui->statusBar->showMessage(tr("%1 entries read, %2 entries processed").arg(read).arg(processed));
        this->checkIfReadyForOp();
    }
}

void MainWindow::deleteDBEntry() {
    QModelIndexList sil = this->ui->tableView->selectionModel()->selectedRows();

    for (int i=0; i<sil.length(); ++i) {
        this->musicDataModel->removeRow(sil.at(i).row(), QModelIndex());
    }

    this->ui->tableView->setUpdatesEnabled(true);
    this->ui->tableView->update(QModelIndex());
}

void MainWindow::dispatchAutotag() {
    QList<QModelIndex> selectedRowList = this->ui->tableView->selectionModel()->selectedRows();

    if (selectedRowList.size()) {
        for (int i=0; i < selectedRowList.size(); ++i) {
            this->fileHandler->autoTagEntry(selectedRowList.at(i));
        }
    }
}

/* =========================================== */
/*       Definition of private functions       */
/* =========================================== */
void MainWindow::processFile(const QString &fpath, unsigned int &proc) {
    if (fpath.contains(this->supportedFiletypes)) {
        QFileInfo fi(fpath);
        if (fi.isReadable()) {
            proc += this->musicDataModel->addFile(fpath);
            return;
        } else {
            this->ui->statusBar->showMessage(tr("File '%1' is unreadable!").arg(fpath));
        }
    } else {
        this->ui->statusBar->showMessage(tr("File '%1' is in the wrong file format!").arg(fpath));
    }
}

void MainWindow::checkIfReadyForOp(void) {
    this->ui->beginSortButton->setEnabled(this->musicDataModel->isReady());
}
