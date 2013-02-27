#ifndef MAINWINDOW_H
#define MAINWINDOW_H

# include <QMainWindow>
# include <QDropEvent>

# include "patternvalidator.h"
# include "filehandler.h"
# include "authordialog.h"
# include "resultdialog.h"
# include "musicdatamodel.h"
# include "musictableeditdelegate.h"
# include "brainzlineedit.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    /* methos    ============================= */
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString expandExamplePattern();

    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *event);

    /* signals and slots ===================== */
signals:
    void addFilesComplete();

public slots:
    void reactOnPatternChange(QString p);
    void setDestPath(void);
    void showAboutQt(void);
    void editSourceFiles(void);
    void cleanup(void);

    /* on the tag tab */
    void addToDB(void);
    void deleteDBEntry(void);
    
private:
    void processFile(const QString& fpath);
    void checkIfReadyForOp(void);

    /* attributes ============================ */
public:
    const QRegExp        supportedFiletypes;
    unsigned int         entriesRead; /* counter of how many directory entries have been read */

private:
    PatternValidator    *patternValidator;
    FileHandler          *fileHandler;

    Ui::MainWindow          *ui;
    AuthorDialog            *authorDialog;
    ResultDialog            *resultDialog;
    MusicDataModel          *musicDataModel;
    MusicTableEditDelegate *musicEditDelegate;
    BrainzLineEdit          *brainzLineEdit;
};

#endif // MAINWINDOW_H
