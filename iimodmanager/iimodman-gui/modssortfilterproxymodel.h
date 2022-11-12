#ifndef MODSSORTFILTERPROXYMODEL_H
#define MODSSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace iimodmanager {

/** Sort/Filter proxy with column-specific behavior for the ModsModel.
 *
 * Version and date columns use custom sorting.
 *
 * Filter strings query both mod name and ID.
 */
class ModsSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ModsSortFilterProxyModel(QObject *parent = nullptr);

public slots:
    void setFilterStatus(ModsModel::Status status);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    ModsModel::Status filterStatus;
};

} // namespace iimodmanager

#endif // MODSSORTFILTERPROXYMODEL_H
