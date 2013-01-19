#ifndef SOURCEFILEEDITDIALOG_H
#define SOURCEFILEEDITDIALOG_H

# include <QDialog>
# include <QStringListModel>
# include <QDragEnterEvent>
# include <QDropEvent>

namespace Ui {
    class SourceFileEditDialog;
}

class SourceFileEditDialog : public QDialog
{
    Q_OBJECT
    
    /* methods =============================== */
public:
    explicit SourceFileEditDialog(QWidget* parent = 0);
    ~SourceFileEditDialog();

    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *event);
    
    void setStringList(const QStringList& sl);
    const QStringList& getStringList(void) const;
    /* slots ================================= */
private slots:
    void addListEntry(void);
    void removeListEntry(void);

private:
    void checkAndAddFile(const QString &fpath);

    /* attributes ============================ */
private:
    Ui::SourceFileEditDialog *ui;

    QStringList        striLi;
    QStringListModel *striLiMod;

    unsigned int entriesRead;
};

#endif // SOURCEFILEEDITDIALOG_H
