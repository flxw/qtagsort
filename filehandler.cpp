# include "filehandler.h"

# include <QDir>
# include <QFile>


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

void FileHandler::setTargetDir(const QString &t) {
    this->targetDir = t;
}
/* =========================================== */
/*      implementation of public slots         */
/* =========================================== */
void FileHandler::startSortAction(void) {
    QStringList::Iterator strIt = this->sourceFileList.begin();
    QString expandedPattern, curLoc;

    QFile file;
    QFileInfo fileInfo;
    QDir directory;

    this->filesCopied  = 0;
    this->copiesFailed = 0;
    const float perc = 100/this->sourceFileList.length();

    for(int i = 1; strIt != this->sourceFileList.end(); ++strIt, ++i) {
        file.setFileName(*strIt);
        fileInfo.setFile(file);
        TagLib::FileRef fr(strIt->toStdString().c_str(), false);

        /* check for nonexistent tags and empty files */
        if (fr.isNull()) {
            ++this->copiesFailed;
            continue;
        } else if (fr.tag()->isEmpty()) {
            ++this->copiesFailed;
            continue;
        }

        /* expand pattern get absolute filepath or directory - depending on mode */
        expandedPattern = (this->renameFiles) ? this->expandPattern(fr, fileInfo) : \
                                                this->expandPattern(fr,fileInfo).append(fileInfo.fileName());
        curLoc          = fileInfo.absoluteFilePath();

        if (expandedPattern != this->pattern && expandedPattern != curLoc) {
            /* find the last directory delimiter */
            const unsigned int pos = expandedPattern.lastIndexOf("/");

            /* create the directory and move the file */
            if (directory.mkpath(expandedPattern.left(pos))) {
                if (file.copy(expandedPattern)) {
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
QString FileHandler::expandPattern(const TagLib::FileRef &fr, const QFileInfo &fI) {
    QString exp = this->pattern;
    exp.replace("/", "\n"); // workaround for slashes in tags - replace the path slashes with NULL

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
        exp.replace(".ext", fI.suffix().prepend("."));

    /* check if part of the pattern could not be expanded */
    if (exp.contains("%"))
        return this->pattern;
    else /* replace tag slashes with underscore and rereplace NULLs with slashes */
        return this->targetDir + "/" + exp.replace("/", "_").replace("\n", "/");
}
