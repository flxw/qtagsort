#ifndef MUSICTABLEEDITDELEGATE_H
#define MUSICTABLEEDITDELEGATE_H

# include <QItemDelegate>
# include <QAbstractItemModel>
# include "brainzlineedit.h"

class MusicTableEditDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit MusicTableEditDelegate(QAbstractItemModel *mod, QObject *parent = 0);

    // reimplemented virtual functions
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QAbstractItemModel *musicdb;
};

#endif // MUSICTABLEEDITDELEGATE_H
