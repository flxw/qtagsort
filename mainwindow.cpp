# include "mainwindow.h"
# include "ui_mainwindow.h"

# include <QFileDialog>
# include <QUrl>
# include <QDirIterator>
# include <QMessageBox>

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
    this->entriesRead      = 0;

    this->ui->patternEdit->setValidator(this->patternValidator);
    this->ui->tab2_tableView->setModel(this->musicDataModel);
    this->ui->tab2_tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    /* signal handler setup ================== */
    /* UI -- basic settings tab */
    connect(this->ui->patternEdit, SIGNAL(textChanged(QString)), this, SLOT(reactOnPatternChange(QString)));
    connect(this->ui->sTargetButton, SIGNAL(clicked()), this, SLOT(setDestPath()));
    /* UI -- basic settings tab */
    //connect(this->ui->tab2_addButton, SIGNAL(clicked()), this, SLO()
    connect(this->ui->tab2_delButton, SIGNAL(clicked()), this, SLOT(deleteDBEntry()));
    /* other connections */
    connect(this->ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(this->ui->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(showAboutQt()));
    connect(this->ui->actionAuthor, SIGNAL(triggered()), this->authorDialog, SLOT(show()));
    connect(this->ui->beginSortButton, SIGNAL(clicked()), this->fileHandler, SLOT(startSortAction()));

    connect(this->fileHandler, SIGNAL(handleProgressPerc(int)), this->ui->progressBar, SLOT(setValue(int)));
    connect(this->fileHandler, SIGNAL(finished(FileHandler::HandleReport)), \
            this->resultDialog, SLOT(prepareShow(FileHandler::HandleReport)));
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

    pattern.replace(MusicDataModel::PH_ARTIST, "Artist");
    pattern.replace(MusicDataModel::PH_ALBUM, "Album");
    pattern.replace(MusicDataModel::PH_TITLE, "Track");
    pattern.replace(MusicDataModel::PH_TRACKNO, "Track Number");
    pattern.replace(MusicDataModel::PH_YEAR, "Year");

    return pattern;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev) {
    /* TODO: check MIME type here */
    ev->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mD = event->mimeData();

    /* check if we can get the location of the files dragged into the window */
    if (mD->hasUrls()) {
        QList<QUrl> urlList = mD->urls();

        QString tempFilename;

        /* check if local file or folder */
        for (int i=0; i < urlList.size(); ++i) {
            tempFilename = urlList.at(i).toLocalFile();

            /* check wether the path leads to a file */
            if (tempFilename.contains(this->supportedFiletypes)) {
                ++this->entriesRead;
                this->processFile(tempFilename);
            } else {
                /* the path leads to a directory, walk it! */
                QDirIterator dirIt(tempFilename, QDirIterator::Subdirectories);

                while ( dirIt.hasNext() ) {
                    dirIt.next();

                    ++this->entriesRead;
                    if ( dirIt.fileInfo().isFile() ) {
                        this->processFile(dirIt.filePath());
                    }
                }
            }
        }

        /* now update the counters */
        this->ui->processableDispLabel->setText(QString("%1").arg(this->musicDataModel->rowCount(QModelIndex())));
        this->ui->entriesReadDispLabel->setText(QString("%1").arg(this->entriesRead));

        /* check if system has enough data */
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
    }
}

void MainWindow::showAboutQt() {
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::editSourceFiles() {
    return;
}

void MainWindow::cleanup() {
    this->musicDataModel->clearData();

    this->entriesRead = 0;
    this->ui->processableDispLabel->setText(QString("0"));
    this->ui->entriesReadDispLabel->setText(QString("0"));
    this->ui->progressBar->setValue(0);

    this->checkIfReadyForOp();
}

/* > on the tag tab */
void MainWindow::addToDB() {
    // select either files or folders with dialog and
    // add like added with DragEvent
    QFileDialog fd(this, tr("Select the files that you want to sort by tags"));
    fd.setFileMode(QFileDialog::ExistingFiles | QFileDialog::Directory);

    if (fd.exec() == QFileDialog::Accepted) {
        QStringList strl = fd.selectedFiles();
        QStringList::iterator strlIt = strl.begin();

        for (; strlIt != strl.end(); ++strlIt) {
            if (strlIt->contains(this->supportedFiletypes)) {
                ++this->entriesRead;
                this->processFile(*strlIt);
            } else {
                /* the path leads to a directory, walk it! */
                QDirIterator dirIt(*strlIt, QDirIterator::Subdirectories);

                while ( dirIt.hasNext() ) {
                    dirIt.next();

                    ++this->entriesRead;
                    if ( dirIt.fileInfo().isFile() ) {
                        this->processFile(dirIt.filePath());
                    }
                }
            }
        }
    }
}

void MainWindow::deleteDBEntry() {
    QModelIndexList sil = this->ui->tab2_tableView->selectionModel()->selectedRows();

    for (unsigned int i=0; i<sil.length(); ++i) {
        this->musicDataModel->removeRow(sil.at(i).row(), QModelIndex());
    }

    this->ui->tab2_tableView->update(QModelIndex());
}

void MainWindow::updateEntry() {
    // update entry using MusicBrainz Database....
    // get data from db, make call to brainz db
    // and then display options
    QModelIndexList sil = this->ui->tab2_tableView->selectionModel()->selectedRows();

    for (int i = 0; i<sil.length(); ++i) {
        this->musicDataModel->updateRowTags(sil.at(i).row());
    }

    this->ui->tab2_tableView->update(QModelIndex());
}

/* =========================================== */
/*       Definition of private functions       */
/* =========================================== */
void MainWindow::processFile(const QString &fpath) {
    if (fpath.contains(this->supportedFiletypes)) {
        QFileInfo fi(fpath);
        if (fi.isReadable()) {
            this->musicDataModel->addFile(fpath);
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
