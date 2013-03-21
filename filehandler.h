#ifndef FILEHANDLER_H
#define FILEHANDLER_H

# include <QObject>
# include <QStringList>

# include <taglib/fileref.h>
# include <taglib/tag.h>

# include "musicdatamodel.h"
# include "fingerprinter.h"


class FileHandler : public QObject
{
    Q_OBJECT

public:
    enum HandleResult {FGOOD, FEMPTY, FTEMPTY, FBAD};

    struct HandleReport {
        unsigned int handled;
        unsigned int failed;
    };

/* =========================================== */
/*           methods are defined below         */
public:
    explicit FileHandler(MusicDataModel *datm, QObject *parent = 0);

    void autoTagEntry(const QModelIndex &mdi);

    /* signals & slots =========================== */
signals:
    void fileHandled(FileHandler::HandleResult);
    void handleProgressPerc(int);
    void finished(FileHandler::HandleReport);
    void status(QString, int);
    void relayedMatches(QStringList, QStringList, QStringList);

public slots:
    void startSortAction(void);

/* =========================================== */
/*        attributes are defined below         */
private:
    /* variables for core functionality */
    MusicDataModel *dm;
    Fingerprinter fp;

    unsigned int filesCopied, copiesFailed;
    bool          renameFiles;
};

#endif // FILEHANDLER_H
