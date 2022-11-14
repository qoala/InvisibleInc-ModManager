#ifndef MODSSORTFILTERPROXYMODEL_H
#define MODSSORTFILTERPROXYMODEL_H

#include "modelutil.h"

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

    //! The columns that will be searched when setFilterFixedString is called (default: None).
    void setFilterTextColumns(const QVector<int> &columns);
    //! The column that will be checked for filter status (default: 0).
    void setFilterStatusColumn(int column);

public slots:
    void setFilterStatus(modelutil::Status requiredStatuses, modelutil::Status maskedStatuses);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QVector<int> filterTextColumns;
    int filterStatusColumn;
    modelutil::Status requiredStatuses;
    modelutil::Status maskedStatuses;
};

} // namespace iimodmanager

#endif // MODSSORTFILTERPROXYMODEL_H
