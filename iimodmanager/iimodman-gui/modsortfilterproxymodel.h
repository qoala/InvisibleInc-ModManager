#ifndef MODSORTFILTERPROXYMODEL_H
#define MODSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace iimodmanager {

/** Sort/Filter proxy with column-specific behavior for the ModCacheModel.
 *
 * Version and date columns use custom sorting.
 *
 * Filter strings query both mod name and ID.
 */
class ModSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ModSortFilterProxyModel(QObject *parent = nullptr);

public slots:
    void setFilterStatus(ModCacheModel::Status status);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    ModCacheModel::Status filterStatus;
};

} // namespace iimodmanager

#endif // MODSORTFILTERPROXYMODEL_H
