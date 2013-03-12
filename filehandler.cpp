# include "filehandler.h"

# include <QDir>
# include <QFile>

FileHandler::FileHandler(MusicDataModel *datm, QObject *parent) : QObject(parent) {
    this->dm = datm;

    // Signals and Slots
    connect(&fp, SIGNAL(status(QString, int)), this, SIGNAL(status(QString, int)));
    connect(&fp, SIGNAL(receivedGoodAnswer(QStringList,QStringList,QStringList)),
            this, SIGNAL(relayedMatches(QStringList,QStringList,QStringList)));
}

/* =========================================== */
/*      implementation of public functions     */
/* =========================================== */
void FileHandler::autoTagEntry(const QModelIndex &mdi) {
    QString filename = this->dm->getFileLocation(mdi);
    fp.getMusicBrainzData(filename);
}

/* =========================================== */
/*      implementation of public slots         */
/* =========================================== */
void FileHandler::startSortAction(void) {
    QList<MusicDataModel::MusicFileData>::const_iterator dbIt = this->dm->getDBstart();

    QFile file;
    QDir directory;

    this->filesCopied  = 0;
    this->copiesFailed = 0;
    const float perc  = 100/this->dm->rowCount(QModelIndex());

    /* everything is initalized, now let the pattern expansion begin */
    this->dm->prepareData();

    for (int i = 1; dbIt != this->dm->getDBend(); ++dbIt, ++i) {
        if (dbIt->isGood) {
            /* find the last directory delimiter */
            file.setFileName(dbIt->location);
            const unsigned int pos = dbIt->destination.lastIndexOf("/");

            /* create the directory and move the file */
            if (directory.mkpath(dbIt->destination.left(pos))) {
                if (file.copy(dbIt->destination)) {
                    ++this->filesCopied;
                } else {
                    ++this->copiesFailed;
                }
            } else {
                ++this->copiesFailed;
            }
        }
        emit handleProgressPerc((int)(perc*i));
    }
    emit handleProgressPerc(100);
    FileHandler::HandleReport hrep;
    hrep.failed  = this->copiesFailed;
    hrep.handled = this->filesCopied;
    emit finished(hrep);
}

/* =========================================== */
/*      implementation of private functions    */
/* =========================================== */
