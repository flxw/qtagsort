#ifndef FILEHANDLER_H
#define FILEHANDLER_H

# include <QObject>

# include <taglib/fileref.h>
# include <taglib/tag.h>

# include "musicdatamodel.h"

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

signals:
    void fileHandled(FileHandler::HandleResult);
    void handleProgressPerc(int);
    void finished(FileHandler::HandleReport);

public slots:
    void startSortAction(void);

/* =========================================== */
/*        attributes are defined below         */
private:
    /* variables for core functionality */
    MusicDataModel *dm;

    unsigned int filesCopied, copiesFailed;
    bool          renameFiles;
};

#endif // FILEHANDLER_H
