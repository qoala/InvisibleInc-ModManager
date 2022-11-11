#ifndef MODSORTFILTERPROXYMODEL_H
#define MODSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace iimodmanager {

/** Sort/Filter proxy with column-specific sorting behavior for the ModCacheModel. */
class ModSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ModSortFilterProxyModel(QObject *parent = nullptr);

protected:
    // bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
};

} // namespace iimodmanager

#endif // MODSORTFILTERPROXYMODEL_H
