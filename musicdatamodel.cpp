# include "musicdatamodel.h"

# include <QFileInfo>
# include <QVariant>
# include <QtAlgorithms>
# include <QStringList>

# include <taglib/fileref.h>

/* helper functions for sorting */
static bool sortByArtist(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.artist.toLower() < rhs.artist.toLower();
}
static bool sortByRelease(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.release.toLower() < rhs.artist.toLower();
}
static bool sortByTitle(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.title.toLower() < rhs.title.toLower();
}
static bool sortByTrackno(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.trackno < rhs.trackno;
}
static bool sortByYear(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.year < rhs.year;
}
static bool sortRevByArtist(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.artist.toLower() > rhs.artist.toLower();
}
static bool sortRevByRelease(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.release.toLower() > rhs.artist.toLower();
}
static bool sortRevByTitle(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.title.toLower() > rhs.title.toLower();
}
static bool sortRevByTrackno(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.trackno > rhs.trackno;
}
static bool sortRevByYear(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    return lhs.year > rhs.year;
}

/* initialize placeholders */
const QString MusicDataModel::PH_ARTIST  = QString("%a");
const QString MusicDataModel::PH_ALBUM   = QString("%r");
const QString MusicDataModel::PH_TITLE   = QString("%t");
const QString MusicDataModel::PH_TRACKNO = QString("%n");
const QString MusicDataModel::PH_YEAR    = QString("%y");

MusicDataModel::MusicDataModel(QObject *parent) : QAbstractTableModel(parent) {
}

/* =========================================== */
/*       Definition  of public functions       */
/* =========================================== */
QList<MusicDataModel::MusicFileData>::const_iterator MusicDataModel::getDBstart(void) const {
    return this->db.begin();
}

QList<MusicDataModel::MusicFileData>::const_iterator MusicDataModel::getDBend(void) const {
    return this->db.end();
}

void MusicDataModel::deactivateDuplicates(const DupResolutionCriteria &criteria) {
    int i = 0;
    QList<QStringList> dList;
    QList< QPair<QString, QPair<int, int> > > dbPairMirrorList; /* index and crit comparison val in second pair */
    QHash<QString, QPair<int,int> > bestForDestination; /* map destination to index and crit comparison value*/

    /* initialize a pair list that mirrors the destinations/indices inside our database */
    for (QList<MusicFileData>::const_iterator it = this->db.begin(); it != this->db.end(); ++it, ++i) {
        if (it->isGood) {
            switch(criteria) {
            case BITRATE:
                dbPairMirrorList.append(QPair<QString,QPair<int,int> >(it->destination, QPair<int,int>(i, it->bitrate)));
                break;

            case DURATION:
                dbPairMirrorList.append(QPair<QString,QPair<int,int> >(it->destination, QPair<int,int>(i, it->duration)));
                break;

            case SAMPLERATE:
                dbPairMirrorList.append(QPair<QString,QPair<int,int> >(it->destination, QPair<int,int>(i, it->samplerate)));
                break;
            }
        }
    }

    /* now, traverse the pairlist to find the duplicates */
    for (QList< QPair<QString,QPair<int,int> > >::const_iterator it = dbPairMirrorList.begin(), cit;
         it != dbPairMirrorList.end(); ++it) {

        /* traverse from current index until the end */
        for (cit = it; cit != dbPairMirrorList.end(); ++cit) {
            /* act on destination match */
            QString f = it->first;

            if (f == cit->first) {
                if (bestForDestination.value(f, QPair<int,int>(-1,-1)).second < cit->second.second) {
                    if (bestForDestination.contains(f)) {
                        this->db[bestForDestination[f].first].isGood = false;
                        bestForDestination[f].first  = cit->second.first;
                        bestForDestination[f].second = cit->second.second;
                        dbPairMirrorList.removeOne(*cit);
                        cit = it;
                    } else {
                        bestForDestination[f].first  = cit->second.first;
                        bestForDestination[f].second = cit->second.second;
                    }
                } else {
                    this->db[cit->second.first].isGood = false;
                    dbPairMirrorList.removeOne(*cit);
                    cit = it;
                }
            }
        }
    }
}

QString MusicDataModel::getFileLocation(const QModelIndex &mdi) const {
    return this->db.at(mdi.row()).location;
}

int MusicDataModel::addFile(const QString &file) {
    TagLib::FileRef fr(file.toStdString().c_str(), true);
    bool isDuplicate = false;

    /* iterate through the whole database and find out whether this
     * file is already inside */
    for (QList<MusicFileData>::const_iterator it = this->db.begin();
         it != this->db.end() && !isDuplicate;
         ++it) {
        isDuplicate = (it->location == file);
    }

    if (fr.isNull() || fr.tag()->isEmpty() || isDuplicate) return 0;

    emit beginInsertRows(QModelIndex(), this->db.size()+1, this->db.size()+1);

    this->db.append(MusicFileData());
    MusicFileData& fileData = this->db.last();

    fileData.location   = file;
    fileData.artist     = QString(fr.tag()->artist().toCString(true));
    fileData.release    = QString(fr.tag()->album().toCString(true));
    fileData.title      = QString(fr.tag()->title().toCString(true));
    fileData.trackno    = fr.tag()->track();
    fileData.year       = fr.tag()->year();
    fileData.samplerate = fr.audioProperties()->sampleRate();
    fileData.bitrate    = fr.audioProperties()->bitrate();
    fileData.duration   = fr.audioProperties()->length();
    fileData.tagsEdited = false;
    fileData.isGood     = false;

    emit endInsertRows();

    return 1;
}

bool MusicDataModel::isReady(void) {
    return !this->db.isEmpty() && !this->pattern.isEmpty() && !this->targetDirectory.isEmpty();
}


void MusicDataModel::setPattern(const QString &p) {
    this->pattern     = p;
    this->renameFiles = this->pattern.endsWith(".ext");
}

void MusicDataModel::setTargetDir(const QString &t) {
    this->targetDirectory = t;
}

void MusicDataModel::reset(void) {
    emit beginResetModel();
    this->db.clear();
    emit endResetModel();
}

void MusicDataModel::prepareData(void) {
    /* prepares the data for the sort operation
     * by setting the location member of all MusicFileInfo elements in this->db  */
    QList<MusicDataModel::MusicFileData>::iterator dbIt = this->db.begin();

    for (; dbIt != this->db.end(); ++dbIt) {
        this->expandPattern(*dbIt);

        if (dbIt->location == dbIt->destination) {
            dbIt->isGood = false;
        }
    }
}


// reimplemented virtual functions ---------------
void MusicDataModel::sort(int column, Qt::SortOrder order) {
    bool (*sortFunc)(const MusicDataModel::MusicFileData&, const MusicDataModel::MusicFileData&);
    bool reverse = (order == Qt::DescendingOrder);

    switch (column) {
        case 0: sortFunc = (reverse)?sortRevByArtist:sortByArtist; break;
        case 1: sortFunc = (reverse)?sortRevByRelease:sortByRelease; break;
        case 2: sortFunc = (reverse)?sortRevByTitle:sortByTitle; break;
        case 3: sortFunc = (reverse)?sortRevByTrackno:sortByTrackno; break;
        case 4: sortFunc = (reverse)?sortRevByYear:sortByYear; break;
        default: sortFunc = NULL; break;
    }

    if (!sortFunc) return;

    qSort(db.begin(), db.end(), sortFunc);
    emit dataChanged(QModelIndex(),QModelIndex());
}

int MusicDataModel::rowCount(const QModelIndex &parent) const {
    return this->db.size();
}

int MusicDataModel::columnCount(const QModelIndex &parent) const {
    return 5;
}

QVariant MusicDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
    QVariant retVariant(QVariant::String);

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: retVariant.setValue(tr("Artist")); break;
        case 1: retVariant.setValue(tr("Album")); break;
        case 2: retVariant.setValue(tr("Title")); break;
        case 3: retVariant.setValue(tr("Tracknumber")); break;
        case 4: retVariant.setValue(tr("Year")); break;
        default: retVariant = QVariant(QVariant::Invalid);
        }
    }

    return retVariant;
}

QVariant MusicDataModel::data(const QModelIndex &index, int role) const {
    /* avoid tableView quirks by only delivering data on specific roles */
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    QVariant retVariant(QVariant::String);

    switch (index.column()) {
    case 0: retVariant.setValue(this->db.at(index.row()).artist); break;
    case 1: retVariant.setValue(this->db.at(index.row()).release); break;
    case 2: retVariant.setValue(this->db.at(index.row()).title); break;
    case 3: retVariant.setValue(this->db.at(index.row()).trackno); break;
    case 4: retVariant.setValue(this->db.at(index.row()).year); break;
    default: retVariant = QVariant(QVariant::Invalid);
    }

    return retVariant;
}

bool MusicDataModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        bool retVal = false;
        unsigned int y;

        switch(index.column()) {
        case 0: this->db[index.row()].artist  = value.toString(); break;
        case 1: this->db[index.row()].release = value.toString(); break;
        case 2: this->db[index.row()].title   = value.toString(); break;
        case 3: y = value.toInt(&retVal); if (retVal) this->db[index.row()].trackno = y; break;
        case 4: y = value.toInt(&retVal); if (retVal) this->db[index.row()].year = y; break;
        default: break;
        }

        if (retVal) {
            emit dataChanged(index, index);
            this->db[index.row()].tagsEdited = true;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool MusicDataModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) {
    return false;
}

bool MusicDataModel::removeRow(int row, const QModelIndex &parent) {
    emit beginRemoveRows(QModelIndex(), row, row);
    this->db.removeAt(row);
    emit endRemoveRows();

    return true;
}

Qt::ItemFlags MusicDataModel::flags(const QModelIndex &index) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
/* =========================================== */
/*       Definition  of public slots           */
/* =========================================== */
/* =========================================== */
/*       Definition  of private functions      */
/* =========================================== */
void MusicDataModel::expandPattern(MusicFileData &mfd) {
    QString exp = this->pattern;

    /* TODO: fix directory seperator for non-UNIX platforms */
    exp.replace("/", "\n"); // workaround for slashes in tags - replace the path slashes with \n

    if (!mfd.artist.isEmpty())
        exp.replace(MusicDataModel::PH_ARTIST, mfd.artist);

    if (!mfd.release.isEmpty())
        exp.replace(MusicDataModel::PH_ALBUM, mfd.release);

    if (!mfd.title.isEmpty())
        exp.replace(MusicDataModel::PH_TITLE, mfd.title);

    if (mfd.trackno != 0)
        exp.replace(MusicDataModel::PH_TRACKNO, QString("%1").arg(mfd.trackno));

    if (mfd.year != 0)
        exp.replace(MusicDataModel::PH_YEAR, QString("%1").arg(mfd.trackno));

    if (this->renameFiles) {
        exp.replace(".ext", mfd.location.right(4));
    } else {
        QFileInfo fi(mfd.location);
        exp.append(fi.fileName());
    }

    /* check if part of the pattern could not be expanded */
    if (exp.contains("%")) {
        mfd.isGood = false;
    } else {
        /* replace tag slashes with underscore and rereplace \n's with slashes */
        mfd.destination = this->targetDirectory + "/" + exp.replace("/","_").replace("\n", "/");
        mfd.isGood = true;
    }
}
