# include "mainwindow.h"
# include "ui_mainwindow.h"

# include "authordialog.h"

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
    this->fileHandler      = new FileHandler(this);
    this->authorDialog     = new AuthorDialog(this);
    this->resultDialog     = new ResultDialog(this);
    this->sourceDialog     = new SourceFileEditDialog(this);

    this->ui->patternEdit->setValidator(this->patternValidator);
    this->entriesRead        = 0;

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
    /* source edit button */
    connect(this->ui->editSourcesButton, SIGNAL(clicked()), this, SLOT(editSourceFiles()));
    /* progress bar updates */
    connect(this->fileHandler, SIGNAL(handleProgressPerc(int)), this->ui->progressBar, SLOT(setValue(int)));
    /* cleanup slot to prepare for next operation and data display */
    connect(this->fileHandler, SIGNAL(finished(FileHandler::HandleReport)), \
            this->resultDialog, SLOT(prepareShow(FileHandler::HandleReport)));
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

    pattern.replace(FileHandler::PH_ARTIST, "Artist");
    pattern.replace(FileHandler::PH_ALBUM, "Album");
    pattern.replace(FileHandler::PH_TITLE, "Track");
    pattern.replace(FileHandler::PH_TRACKNO, "Track Number");
    pattern.replace(FileHandler::PH_YEAR, "Year");

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
        this->ui->processableDispLabel->setText(QString("%1").arg(this->fileHandler->getSources().count()));
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
        this->fileHandler->setPattern(p);
        this->ui->parsedPatternDispLabel->setText(this->expandExamplePattern());
    } else {
        this->fileHandler->setPattern(QString());
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
        this->fileHandler->setTargetDir(dest);
        this->ui->targetDispLabel->setText(dest);
        this->checkIfReadyForOp();
    }
}

void MainWindow::showAboutQt() {
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::editSourceFiles() {
    this->sourceDialog->setStringList(this->fileHandler->getSources());
    this->sourceDialog->show();

    /* if dialog accepted, set filehandler sources */
    //if (this->sourceDialog->)
}

void MainWindow::cleanup() {
    this->fileHandler->setSources(QStringList());
    this->fileHandler->setTargetDir(QString());
    this->fileHandler->setPattern(QString());

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
            this->fileHandler->addToSources(fpath);
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
     *      - pattern set
     *      - input files given
     */
    if ( this->fileHandler->isReady() ) {
        this->ui->beginSortButton->setEnabled(true);
    } else {
        this->ui->beginSortButton->setDisabled(true);
    }
}
