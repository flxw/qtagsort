#ifndef PROPOSALSELECTIONDIALOG_H
#define PROPOSALSELECTIONDIALOG_H

# include <QDialog>
# include <QStringListModel>

namespace Ui {
class ProposalSelectionDialog;
}

class ProposalSelectionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ProposalSelectionDialog(QStringList tl, QStringList rl, QStringList al, QString filename, QWidget *parent = 0);
    ~ProposalSelectionDialog();

    QString getTitleSelection();
    QString getReleaseSelection();
    QString getArtistSelection();
    
private:
    Ui::ProposalSelectionDialog *ui;
    QStringListModel *titleListModel, *artistListModel, *releaseListModel;
};

#endif // PROPOSALSELECTIONDIALOG_H
