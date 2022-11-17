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

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    //! Overridden just to apply CANCEL_SORT_ROLE.
    //! Sorting is cancelled until the next time ::sort is called.
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    //! The columns that will be searched when setFilterFixedString is called (default: None).
    void setFilterTextColumns(const QVector<int> &columns);
    //! The column that will be checked for filter status (default: 0).
    void setFilterStatusColumn(int column);

public slots:
    void setFilterStatus(modelutil::Status requiredStatuses, modelutil::Status maskedStatuses);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private slots:
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    QVector<int> filterTextColumns;
    int filterStatusColumn;
    modelutil::Status requiredStatuses;
    modelutil::Status maskedStatuses;
    bool sortCancelled;
};

} // namespace iimodmanager

#endif // MODSSORTFILTERPROXYMODEL_H
