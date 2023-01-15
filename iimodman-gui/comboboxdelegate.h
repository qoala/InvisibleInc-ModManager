#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include "modsmodel.h"

#include <QAbstractItemModel>
#include <QStyledItemDelegate>

namespace iimodmanager {

class ModVersionComboBoxDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    ModVersionComboBoxDelegate(ModsModel *mods, int valueColumn, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
            const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;

private:
    ModsModel *mods;
    const int valueColumn;
};

}  // namespace iimodmanager

#endif // COMBOBOXDELEGATE_H
