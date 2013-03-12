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

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    /* methods    ============================ */
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString expandExamplePattern();

    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *event);

private:
    void processFile(const QString& fpath, unsigned int &proc);
    void checkIfReadyForOp(void);

    /* signals and slots ===================== */
signals:
    void addFilesComplete();

public slots:
    void reactOnPatternChange(QString p);
    void setDestPath(void);
    void showAboutQt(void);
    void showFileLocation(const QModelIndex &mdi);
    void displayMatchSelectionDialog(QStringList tl, QStringList rl, QStringList al);
    void cleanup(void);

    void addToDB(void);
    void deleteDBEntry(void);
    void dispatchAutotag(void);

    /* attributes ============================ */
public:
    const QRegExp        supportedFiletypes;

private:
    PatternValidator    *patternValidator;
    FileHandler          *fileHandler;

    Ui::MainWindow       *ui;
    AuthorDialog         *authorDialog;
    ResultDialog         *resultDialog;
    MusicDataModel       *musicDataModel;
};

#endif // MAINWINDOW_H
