#ifndef AUTHORDIALOG_H
#define AUTHORDIALOG_H

#include <QDialog>

namespace Ui {
class AuthorDialog;
}

class AuthorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AuthorDialog(QWidget *parent = 0);
    ~AuthorDialog();
    
private:
    Ui::AuthorDialog *ui;
};

#endif // AUTHORDIALOG_H
