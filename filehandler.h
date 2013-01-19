#ifndef FILEHANDLER_H
#define FILEHANDLER_H

# include <QObject>
# include <QStringList>
# include <QFileInfo>

# include <taglib/fileref.h>
# include <taglib/tag.h>

class FileHandler : public QObject
{
    Q_OBJECT

public:
    enum HandleResult {FGOOD, FEMPTY, FTEMPTY, FBAD};
    const static QString PH_ARTIST, PH_ALBUM, PH_TITLE, PH_TRACKNO, PH_YEAR;

    struct HandleReport {
        unsigned int handled;
        unsigned int failed;
    };

/* =========================================== */
/*           methods are defined below         */
public:
    explicit FileHandler(QObject *parent = 0);

    bool isReady(void);

    /* getters and setters */
    void setPattern(const QString &p);
    void setSources(const QStringList &sl);
    void addToSources(const QString &src);
    const QStringList& getSources(void) const;

    void setTargetDir(const QString &t);

signals:
    void fileHandled(FileHandler::HandleResult);
    void handleProgressPerc(int);
    void finished(FileHandler::HandleReport);

public slots:
    void startSortAction(void);

private:
    QString expandPattern(const TagLib::FileRef &fr, const QFileInfo &fI);

/* =========================================== */
/*        attributes are defined below         */
private:
    /* variables for core functionality */
    QStringList  sourceFileList;
    QString       targetDir;
    QString       pattern;

    unsigned int filesCopied, copiesFailed;
    bool          renameFiles;
};

#endif // FILEHANDLER_H
