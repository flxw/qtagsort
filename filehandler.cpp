# include "filehandler.h"

# include <QFile>
# include <QFileInfo>

/* ========================================== */
/* init placeholders */
const QString FileHandler::PH_ARTIST  = QString("%a");
const QString FileHandler::PH_ALBUM   = QString("%r");
const QString FileHandler::PH_TITLE   = QString("%t");
const QString FileHandler::PH_TRACKNO = QString("%n");
const QString FileHandler::PH_YEAR    = QString("%y");


FileHandler::FileHandler(QObject *parent) : QObject(parent) {
    this->renameFiles = false;
}

/* =========================================== */
/*      implementation of public functions     */
/* =========================================== */
bool FileHandler::isReady(void) {
    return (!this->targetDir.isEmpty() && !this->sourceFileList.isEmpty() && !this->pattern.isEmpty());
}

void FileHandler::setPattern(const QString &p) {
    this->pattern = p;

    /* check for file extension at the end */
    this->renameFiles = pattern.endsWith(".ext");
}

void FileHandler::setSources(const QStringList &sl) {
    this->sourceFileList = sl;
}

const QStringList& FileHandler::getSources(void) const{
    return this->sourceFileList;
}

void FileHandler::addToSources(const QString &src) {
    this->sourceFileList.append(src);
}

void FileHandler::setTargetDir(QString &t) {
    this->targetDir = t;
}
/* =========================================== */
/*      implementation of public slots         */
/* =========================================== */
void FileHandler::startSortAction(void) {
    QStringList::Iterator strIt = this->sourceFileList.begin();
    QString expandedPattern, curLoc, fileExtension;
    const double perc = 100/this->sourceFileList.length();

    for(int i = 1; strIt != this->sourceFileList.end(); ++strIt) {
        TagLib::FileRef fr(strIt->toStdString().c_str(), false);
        /* check for nonexistent tags and empty files */
        if (fr.isNull()) {
            emit fileHandled(FileHandler::FEMPTY);
            continue;
        } else if (fr.tag()->isEmpty()) {
            emit fileHandled(FileHandler::FTEMPTY);
            continue;
        }

        /* now read tags and handle data */
        if (this->renameFiles)
            fileExtension   = this->getFileExtension(*strIt);

        expandedPattern = this->expandPattern(fr, fileExtension);
        curLoc          = this->isolateFilePath(*strIt);

        if (expandedPattern != curLoc) {
            /* now move the file */
            QFile file(*strIt);
            if (file.rename(expandedPattern)) {
                emit fileHandled(FileHandler::FGOOD);
            } else {
                emit fileHandled(FileHandler::FBAD);
            }
        }

        emit handleProgressPerc(((int)perc)*i);
    }
}
/* =========================================== */
/*      implementation of private functions    */
/* =========================================== */
QString FileHandler::expandPattern(const TagLib::FileRef &fr, const QString &ext) {
    QString exp = this->pattern;

    if (!fr.tag()->artist().isEmpty())
        exp.replace(FileHandler::PH_ARTIST, fr.tag()->artist().toCString(true));

    if (!fr.tag()->album().isEmpty())
        exp.replace(FileHandler::PH_ALBUM, fr.tag()->album().toCString(true));

    if (!fr.tag()->title().isEmpty())
        exp.replace(FileHandler::PH_TITLE, fr.tag()->title().toCString(true));

    if (fr.tag()->track())
        exp.replace(FileHandler::PH_TRACKNO, QString("%1").arg(fr.tag()->track()));

    if (fr.tag()->year())
        exp.replace(FileHandler::PH_YEAR, QString("%1").arg(fr.tag()->year()));

    if (this->renameFiles)
        exp.replace(".ext", ext);

    /* check if part of the pattern could not be expanded */
    if (exp.contains("%"))
        return this->pattern;
    else
        return this->targetDir + "/" + exp;
}

QString FileHandler::isolateFilePath(const QString &path) {
    QFileInfo fi(path);

    if (this->renameFiles) {
        return fi.absoluteFilePath();
    } else {
        return fi.absolutePath();
    }
}

QString FileHandler::getFileExtension(const QString &fname) {
    QFileInfo fi(fname);

    return fi.suffix().prepend(".");
}
