#ifndef MAINWINDOW_H
#define MAINWINDOW_H

# include <QMainWindow>
# include <QDropEvent>
# include <QButtonGroup>

# include "patternvalidator.h"
# include "fingerprinter.h"
# include "authordialog.h"
# include "resultdialog.h"
# include "musicdatamodel.h"

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
    void dragMoveEvent(QDragMoveEvent *ev);
    void dropEvent(QDropEvent *event);

private:
    void processEntryList(const QStringList &el);
    void processFile(const QString& fpath, int &proc);
    void checkIfReadyForOp(void);

    /* signals and slots ===================== */
signals:
    void addFilesComplete();

public slots:
    void setDestPath(void);
    void reactOnPatternChange(QString p);
    void addToDBviaDialog(QAction *act);
    void deleteDBEntry(void);
    void dispatchAutotag(void);
    void displayMatchSelectionDialog(QString fl, QStringList tl, QStringList rl, QStringList al);
    void startSortAction(void);

    void showAboutQt(void);
    void showFileLocation(const QModelIndex &mdi);

    void cleanup(void);

    /* attributes ============================ */
public:
    const QRegExp        supportedFiletypes;

private:
    PatternValidator *patternValidator;
    Fingerprinter    *fingerprinter;

    Ui::MainWindow   *ui;
    AuthorDialog     *authorDialog;
    ResultDialog     *resultDialog;
    MusicDataModel   *musicDataModel;

    QButtonGroup     *duplicateResolutionGroup;
};

#endif // MAINWINDOW_H
