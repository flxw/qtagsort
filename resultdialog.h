#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

# include <QDialog>

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
    void prepareShow(const int& good, const int& bad);
    
private:
    Ui::ResultDialog *ui;
};

#endif // RESULTDIALOG_H
