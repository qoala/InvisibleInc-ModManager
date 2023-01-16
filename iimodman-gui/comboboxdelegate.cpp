#include "comboboxdelegate.h"
#include "modelutil.h"

#include <QComboBox>
#include <QTimer>

namespace iimodmanager {

ModVersionComboBoxDelegate::ModVersionComboBoxDelegate(ModsModel *mods, int valueColumn, QObject *parent)
    : QStyledItemDelegate(parent), mods(mods), valueColumn(valueColumn)
{}

void ModVersionComboBoxDelegate::commitEditorChange()
{
    QComboBox *editor = qobject_cast<QComboBox*>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

QWidget *ModVersionComboBoxDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->setFrame(false);
    editor->setInsertPolicy(QComboBox::NoInsert);

    const QString modId = index.data(modelutil::MOD_ID_ROLE).toString();
    const QModelIndex rootIndex = mods->indexOfMod(modId);

    if (rootIndex.isValid())
    {
        editor->setModel(mods);
        editor->setModelColumn(valueColumn);
        editor->setRootModelIndex(rootIndex);
        connect(editor, QOverload<int>::of(&QComboBox::activated), this, &ModVersionComboBoxDelegate::commitEditorChange);
        // Open the combobox list, after it has been attached.
        QTimer::singleShot(0, editor, [=](){ editor->showPopup(); });
    }
    return editor;
}

void ModVersionComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    if (value >= 0 && value < comboBox->count())
        comboBox->setCurrentIndex(value);
}

void ModVersionComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = comboBox->currentIndex();
    if (value >= 0)
        model->setData(index, value, Qt::EditRole);
}

void ModVersionComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
        const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

}  // namespace iimodmanager
