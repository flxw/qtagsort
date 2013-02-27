#ifndef BRAINZLINEEDIT_H
#define BRAINZLINEEDIT_H

# include <QLineEdit>
# include <QCompleter>
# include <QStringListModel>

# include <musicbrainz5/ArtistList.h>
# include <musicbrainz5/ReleaseList.h>

# include <map>

class BrainzLineEdit : public QLineEdit {

    Q_OBJECT

public:
    explicit BrainzLineEdit(QWidget *parent = 0);

    void keyPressEvent(QKeyEvent *ev);

    void setActiveColumn(int col);
    void setAdditionalInfo(const QString &a, const QString &r);
    
signals:
    void brainzNotification(QString);
    
public slots:
    void prepareCompleterData(void);

private:
    std::map<std::string, std::string> generateParameterMap(void);
    void loadArtistList(const MusicBrainz5::CArtistList *alist);
    void loadReleaseList(const MusicBrainz5::CReleaseList *rlist);
    //void loadTitleList(const MusicBrainz5::CReleaseList &rlist);


private:
    QCompleter  *_completer;
    QStringList suggestionList;
    QStringListModel* suggestionModel;

    bool isCompleterEnabled;
    int  activeColumn;
    QString artist, release, queryType;
};

#endif // BRAINZLINEEDIT_H
