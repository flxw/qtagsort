# include "musicdatamodel.h"

# include <QFile>
# include <QVariant>
# include <QtAlgorithms>

# include <taglib/fileref.h>

/* helper function for sorting */
/* TODO: make do without static variables, introduce functions like sortByArtist.... */
static int sortCol;
static Qt::SortOrder sortOrder;
static bool sortBySelectedCol(const MusicDataModel::MusicFileData &lhs, const MusicDataModel::MusicFileData &rhs) {
    bool blnRet;

    switch (sortCol) {
    case 0: blnRet = lhs.artist.toLower() < rhs.artist.toLower(); break;
    case 1: blnRet = lhs.release.toLower() < rhs.artist.toLower(); break;
    case 2: blnRet = lhs.title.toLower() < rhs.title.toLower(); break;
    case 3: blnRet = lhs.trackno < rhs.trackno; break;
    case 4: blnRet = lhs.year < rhs.year; break;
    default: blnRet = false; break;
    }

    return (sortOrder == Qt::DescendingOrder)?blnRet:!blnRet;
}

/* initialize placeholders */
const QString MusicDataModel::PH_ARTIST  = QString("%a");
const QString MusicDataModel::PH_ALBUM   = QString("%r");
const QString MusicDataModel::PH_TITLE   = QString("%t");
const QString MusicDataModel::PH_TRACKNO = QString("%n");
const QString MusicDataModel::PH_YEAR    = QString("%y");

MusicDataModel::MusicDataModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

/* =========================================== */
/*       Definition  of public functions       */
/* =========================================== */
void MusicDataModel::addFile(const QString &file) {
    TagLib::FileRef fr(file.toStdString().c_str(), false);

    if (fr.isNull() || fr.tag()->isEmpty()) return;

    emit beginInsertRows(QModelIndex(), this->db.size()+1, this->db.size()+2);

    this->db.append(MusicFileData());
    MusicFileData& fileData = this->db.last();

    fileData.location   = file;
    fileData.artist     = QString(fr.tag()->artist().toCString(true));
    fileData.release    = QString(fr.tag()->album().toCString(true));
    fileData.title      = QString(fr.tag()->title().toCString(true));
    fileData.trackno    = fr.tag()->track();
    fileData.year       = fr.tag()->year();
    fileData.tagsEdited = false;
    fileData.isGood     = false;

    emit endInsertRows();
}

QList<MusicDataModel::MusicFileData>::const_iterator MusicDataModel::getDBstart(void) const {
    return this->db.begin();
}

QList<MusicDataModel::MusicFileData>::const_iterator MusicDataModel::getDBend(void) const {
    return this->db.end();
}

void MusicDataModel::setPattern(const QString &p) {
    this->pattern     = p;
    this->renameFiles = this->pattern.endsWith(".ext");
}

void MusicDataModel::setTargetDir(const QString &t) {
    this->targetDirectory = t;
}

void MusicDataModel::clearData(void) {
    this->db.clear();
    this->pattern = QString();
    this->targetDirectory = QString();
}

void MusicDataModel::prepareData(void) {
    /* prepares the data for the sort operation
     * by setting the location member of all MusicFileInfo elements in this->db
     */
    QList<MusicDataModel::MusicFileData>::iterator dbIt = this->db.begin();

    for (; dbIt != this->db.end(); ++dbIt) {
        this->expandPattern(*dbIt);

        if (dbIt->location == dbIt->destination) {
            dbIt->isGood = false;
        }
    }
}

#include <iostream>
#include <musicbrainz5/Query.h>
#include <musicbrainz5/Metadata.h>
#include <musicbrainz5/ReleaseList.h>
#include <musicbrainz5/Release.h>
#include <musicbrainz5/Artist.h>
#include <musicbrainz5/HTTPFetch.h>
#include <musicbrainz5/Disc.h>
void MusicDataModel::updateRowTags(const int &row) {

    MusicBrainz5::CQuery Query("cdlookupexample-1.0");
    MusicBrainz5::CQuery::tParamMap pmap;
    pmap.insert(std::pair<std::string,std::string>("query", this->db[row].artist.toStdString().c_str()));

    try
    {
        MusicBrainz5::CMetadata Metadata=Query.Query("artist","","",pmap);
        if (Metadata.ArtistList())
        {
            MusicBrainz5::CArtistList *artistList=Metadata.ArtistList();

            std::cout << "Found " << artistList->NumItems() << " artist(s)" << std::endl;
            for (int count=0;count<artistList->NumItems();count++)
            {
                MusicBrainz5::CArtist *art=artistList->Item(count);
                std::cout << "\t" << art->Name() << " -- " << art->ID() << std::endl;

            }
        }
    }

    catch (MusicBrainz5::CConnectionError& Error)
    {
        std::cout << "Connection Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CTimeoutError& Error)
    {
        std::cout << "Timeout Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CAuthenticationError& Error)
    {
        std::cout << "Authentication Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CFetchError& Error)
    {
        std::cout << "Fetch Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CRequestError& Error)
    {
        std::cout << "Request Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CResourceNotFoundError& Error)
    {
        std::cout << "ResourceNotFound Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }
}

bool MusicDataModel::isReady(void) {
    return !this->db.isEmpty() && !this->pattern.isEmpty() && !this->targetDirectory.isEmpty();
}

// required functions ---------------------------
void MusicDataModel::sort(int column, Qt::SortOrder order) {
    sortCol   = column;
    sortOrder = order;

    qSort(db.begin(), db.end(), sortBySelectedCol);
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
        case 0: retVariant.setValue(tr("Arist")); break;
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
    if (role != Qt::DisplayRole) {
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
    this->db.removeAt(row);
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
        QFile mf(mfd.location);
        exp.append(mf.fileName());
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
