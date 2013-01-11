#ifndef MAINWINDOW_H
#define MAINWINDOW_H

# include <QMainWindow>
#include <QDropEvent>

# include "patternvalidator.h"
# include "filehandler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    /* functions ============================= */
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString expandExamplePattern();

    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *event);

public slots:
    void reactOnPatternChange(QString p);
    void setDestPath(void);
    
private:
    void checkAndAddFile(const QString& fpath);
    void checkIfReadyForOp(void);


    /* attributes ============================ */
private:
    int           entriesRead; /* counter of how many directory entries have been read */

    QRegExp      supportedFiletypes;

    PatternValidator *patternValidator;
    FileHandler        *fileHandler;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
