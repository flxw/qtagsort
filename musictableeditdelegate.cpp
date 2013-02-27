# include "musictableeditdelegate.h"

/* Nach 20 Minuten googeln komme ich zu folgendem Ergebnis:
 * QCompleter mal eben neu implementieren - das ist leider nicht drin, pathFromIndex funktioniert nicht!
 *
 * Also: QLineEdit vererben, als Delegate verwenden
 * Dieses LineEdit reagiert selbstständig auf Complete-Anforderungen
 * Es verwaltet eine eigene Datenbank für MusicBrainzCompletions
 * Awesome gell :D. */

MusicTableEditDelegate::MusicTableEditDelegate(QAbstractItemModel *mod, QObject *parent) :
    QItemDelegate(parent)
{
    musicdb = mod;
}

// reimplemented virtual functions
QWidget* MusicTableEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return new BrainzLineEdit((QWidget*)parent);
}

void MusicTableEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    BrainzLineEdit *ede = static_cast<BrainzLineEdit*>(editor);

    ede->setActiveColumn(index.column());
    ede->setAdditionalInfo(index.model()->data(index.child(index.row(), 0)).toString(),
                           index.model()->data(index.child(index.row(), 0)).toString());
    ede->setText(index.model()->data(index, Qt::EditRole).toString());
}

void MusicTableEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    BrainzLineEdit *ed = static_cast<BrainzLineEdit*>(editor);
    model->setData(index, QVariant(ed->text()));
}

void MusicTableEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}
