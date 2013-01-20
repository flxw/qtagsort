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
    this->sourceDialog     = new SourceFileEditDialog(this);
    this->musicDataModel   = new MusicDataModel(this);
    this->fileHandler      = new FileHandler(this->musicDataModel, this);
    this->entriesRead      = 0;

    this->ui->patternEdit->setValidator(this->patternValidator);
    this->ui->tab2_tableView->setModel(this->musicDataModel);
    this->ui->tab2_tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    /* signal handler setup ================== */
    /* ui stuff, actions */
    connect(this->ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(this->ui->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(showAboutQt()));
    connect(this->ui->actionAuthor, SIGNAL(triggered()), this->authorDialog, SLOT(show()));
    /* beginSortButton */
    connect(this->ui->beginSortButton, SIGNAL(clicked()), this->fileHandler, SLOT(startSortAction()));
    /* patternEdit */
    connect(this->ui->patternEdit, SIGNAL(textChanged(QString)), this, SLOT(reactOnPatternChange(QString)));
    /* destination selection button */
    connect(this->ui->sTargetButton, SIGNAL(clicked()), this, SLOT(setDestPath()));
    /* view updates */
    //connect(this->musicDataModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this->ui->tab2_tableView, SLOT(resizeColumnsToContents()));
    /* progress bar updates */
    connect(this->fileHandler, SIGNAL(handleProgressPerc(int)), this->ui->progressBar, SLOT(setValue(int)));
    /* cleanup slot to prepare for next operation and data display */
    /*connect(this->fileHandler, SIGNAL(finished(FileHandler::HandleReport)), \
            this->resultDialog, SLOT(prepareShow(FileHandler::HandleReport)));*/
    connect(this->resultDialog, SIGNAL(finished(int)), this, SLOT(cleanup()));
}

MainWindow::~MainWindow() {
    delete ui;
    delete fileHandler;
    delete patternValidator;
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

/* =========================================== */
/*       Definition of private functions       */
/* =========================================== */
void MainWindow::checkAndAddFile(const QString &fpath) {
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
    /* to be able to start the sorting operation. several things need to be guaranteed:
     *      - target directory set
     *      - pattern is set
     *      - input files given
     */
    this->ui->beginSortButton->setEnabled(this->musicDataModel->isReady());
}
