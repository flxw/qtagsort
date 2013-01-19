#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

# include <QDialog>

# include <filehandler.h>

namespace Ui {
class ResultDialog;
}

class ResultDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ResultDialog(QWidget *parent = 0);
    ~ResultDialog();

public slots:
    void prepareShow(FileHandler::HandleReport res);
    
private:
    Ui::ResultDialog *ui;
};

#endif // RESULTDIALOG_H
