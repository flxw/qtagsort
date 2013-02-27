# include "musictableeditdelegate.h"
# include "mainwindow.h"

MusicTableEditDelegate::MusicTableEditDelegate(QAbstractItemModel *mod, QObject *parent) : QItemDelegate(parent) {
    musicdb = mod;
}

// reimplemented virtual functions
QWidget* MusicTableEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    BrainzLineEdit *bredit = new BrainzLineEdit((QWidget*)parent);

    connect(bredit, SIGNAL(brainzNotification(QString)), this, SLOT(relayBrainzNotification(QString)));

    return bredit;
}

void MusicTableEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    BrainzLineEdit *ede = static_cast<BrainzLineEdit*>(editor);

    qDebug("active column: %i", index.column());
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


/* slot implementation ----------------------- */
void MusicTableEditDelegate::relayBrainzNotification(QString msg) {
    emit relayedBrainzNotification(msg);
}
