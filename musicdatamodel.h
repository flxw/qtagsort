#ifndef MUSICDATAMODEL_H
#define MUSICDATAMODEL_H

# include <QAbstractTableModel>
# include <QList>

/* class abstract
 *  this class shall
     - receive drag and drop inputs
     - read file tags and add them to the index
     - allow tag edits
     - shall provide a basis for plugging in future MusicBrainz support
*/

class MusicDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    const static QString PH_ARTIST, PH_ALBUM, PH_TITLE, PH_TRACKNO, PH_YEAR;
    struct MusicFileData {
        QString location;    /* where the files comes from on the system - ABSOLUTE */
        QString destination; /* where the file will go - ABSOLUTE */

        QString artist;
        QString release;
        QString title;

        unsigned int trackno;
        unsigned int year;

        bool tagsEdited; /* have the tags been edited by the user ? */
        bool isGood;     /* shows us wether the file should be looked at and moved */
    };

    /* methods =============================== */
public:
    explicit MusicDataModel(QObject *parent = 0);

    /* adds a file to the database assuming that permissions and filetype are ok */
    void addFile(const QString &file);
    // void removeFile....
    // void editFile...
    QList<MusicDataModel::MusicFileData>::const_iterator getDBstart(void) const;
    QList<MusicDataModel::MusicFileData>::const_iterator getDBend(void) const;

    void setPattern(const QString &p);
    void setTargetDir(const QString &t);
    void clearData(void);
    void prepareData(void);

    bool isReady(void);
    // required functions
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);
    bool removeRow(int row, const QModelIndex &parent);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    
private:
    void expandPattern(MusicFileData &mfd);
    /* Qt signals and slots ================== */
signals:

public slots:
    
    /* attributes ============================ */
private:
    QString targetDirectory; /* the parent directory of where the files shall go */
    QString pattern;

    bool renameFiles;

    QList<MusicFileData> db;
};

#endif // MUSICDATAMODEL_H
