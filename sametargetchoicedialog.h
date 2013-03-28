#ifndef SAMETARGETCHOICEDIALOG_H
#define SAMETARGETCHOICEDIALOG_H

# include <QDialog>
# include <QStringListModel>

namespace Ui {
class SameTargetChoiceDialog;
}

class SameTargetChoiceDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SameTargetChoiceDialog(const QList<QStringList> &duplicateList, QWidget *parent = 0);
    ~SameTargetChoiceDialog();

    QStringList getBadDuplicates(void);

private slots:
    void changeSourceDisplay(const QModelIndex& newIndex);
    void markChosenAndMoveOn(const QModelIndex& dcIndex);
    
private:
    void checkForCompleteness(void);

private:
    Ui::SameTargetChoiceDialog *ui;

    QList<QStringList> sourceFileList;
    QList<int> chosenFiles;

    QStringListModel *destinationModel;
    QStringListModel *sourceFileModel;

    int currentIndex;
};

#endif // SAMETARGETCHOICEDIALOG_H
