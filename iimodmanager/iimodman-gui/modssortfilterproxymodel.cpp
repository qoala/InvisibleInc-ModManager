#include "modssortfilterproxymodel.h"

#include <QDateTime>
#include <QRegularExpression>

namespace iimodmanager {

typedef modelutil::Status Status;

namespace ColumnLessThan {
    bool modId(const QVariant &leftData, const QVariant &rightData)
    {
        // Sort steam IDs numerically.
        QRegularExpression steamRe(QStringLiteral("^workshop-(\\d+)(\\D.*)?"));
        QRegularExpressionMatch leftMatch = steamRe.match(leftData.toString());
        QRegularExpressionMatch rightMatch = steamRe.match(rightData.toString());
        if (leftMatch.hasMatch() && rightMatch.hasMatch())
        {
            // Steam workshop IDs have crossed the int boundary.
            qulonglong leftNumber = leftMatch.capturedView(1).toULongLong();
            qulonglong rightNumber = rightMatch.capturedView(1).toULongLong();
            if (leftNumber != rightNumber)
                return leftNumber < rightNumber;
            else
                // If a non-steam ID starts with a steam ID, it should sort correctly among them to maintain a total ordering.
                return leftMatch.capturedView(2) < rightMatch.capturedView(2);
        }

        // Sort everything else alphabetically.
        return leftData.toString() < rightData.toString();
    }

    bool modVersion(const QVariant &leftData, Status leftStatus, const QVariant &rightData, Status rightStatus)
    {
        // NULL is less than any other value.
        if (leftStatus.testFlag(modelutil::NULL_STATUS) || rightStatus.testFlag(modelutil::NULL_STATUS))
            return leftStatus.testFlag(modelutil::NULL_STATUS) && !rightStatus.testFlag(modelutil::NULL_STATUS);
        // UNLABELLED is less than any provided value.
        if (leftStatus.testFlag(modelutil::UNLABELLED_STATUS) || rightStatus.testFlag(modelutil::UNLABELLED_STATUS))
            return leftStatus.testFlag(modelutil::UNLABELLED_STATUS) && !rightStatus.testFlag(modelutil::UNLABELLED_STATUS);

        QRegularExpression versionRe(QStringLiteral("^v?(\\d+(?:\\.\\d+)*)([^\\d.]\\S*)?"));
        QRegularExpressionMatch leftMatch = versionRe.match(leftData.toString());
        QRegularExpressionMatch rightMatch = versionRe.match(rightData.toString());
        if (!leftMatch.hasMatch() && !rightMatch.hasMatch())
            return leftData.toString() < rightData.toString();
        else if (leftMatch.hasMatch() != rightMatch.hasMatch())
            return !leftMatch.hasMatch();

        // Parse out individual version number components.
        auto leftNumberParts = leftMatch.capturedView(1).split('.');
        QVector<int> leftNumbers;
        leftNumbers.reserve(leftNumberParts.size());
        for (const auto &p : leftNumberParts)
            leftNumbers << p.toInt();
        auto rightNumberParts = rightMatch.capturedView(1).split('.');
        QVector<int> rightNumbers;
        rightNumbers.reserve(rightNumberParts.size());
        for (const auto &p : rightNumberParts)
            rightNumbers << p.toInt();

        // Compare each component.
        for (int i = 0; i < leftNumbers.size() && rightNumbers.size(); i++)
            if (leftNumbers.at(i) != rightNumbers.at(i))
                return leftNumbers.at(i) < rightNumbers.at(i);
        // Shorter component lists are first.
        if (leftNumbers.size() != rightNumbers.size())
            return leftNumbers.size() < rightNumbers.size();

        return leftMatch.capturedView(2) < rightMatch.capturedView(3);
    }

    bool modVersionTime(const QVariant &leftData, Status leftStatus, const QVariant &rightData, Status rightStatus)
    {
        // NULL is less than any other value.
        if (leftStatus.testFlag(modelutil::NULL_STATUS) || leftStatus.testFlag(modelutil::NULL_STATUS))
            return leftStatus.testFlag(modelutil::NULL_STATUS) && !rightStatus.testFlag(modelutil::NULL_STATUS);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        bool leftIsDateTime = static_cast<QMetaType::Type>(leftData.type()) == QMetaType::QDateTime;
        bool rightIsDateTime = static_cast<QMetaType::Type>(rightData.type()) == QMetaType::QDateTime;
#else
        bool leftIsDateTime = leftData.typeId() == QMetaType::QDateTime;
        bool rightIsDateTime = rightData.typeId() == QMetaType::QDateTime;
#endif
        if (leftIsDateTime && rightIsDateTime)
            return leftData.toDateTime() < rightData.toDateTime();
        else
        {
            // Compare in the same order as the original version IDs. ("dev" is newer than pure dates. "###-original" is older than pure dates.)
            QString left = leftIsDateTime ? leftData.toDateTime().toString(Qt::ISODate) : leftData.toString();
            QString right = rightIsDateTime ? rightData.toDateTime().toString(Qt::ISODate) : rightData.toString();
            return left < right;
        }
    }
}


ModsSortFilterProxyModel::ModsSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent), filterStatusColumn(0), sortCancelled(false) {}

void ModsSortFilterProxyModel::setFilterTextColumns(const QVector<int> &columns)
{
    filterTextColumns = columns;
    invalidateFilter();
}

void ModsSortFilterProxyModel::setFilterStatusColumn(int column)
{
    filterStatusColumn = column;
    invalidateFilter();
}

void ModsSortFilterProxyModel::setFilterStatus(modelutil::Status requiredStatuses, modelutil::Status maskedStatuses)
{
    this->requiredStatuses = requiredStatuses;
    this->maskedStatuses = maskedStatuses;
    invalidateFilter();
}

bool ModsSortFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // Cancel sorting.
    QVariant cancelColumnsData = sourceModel()->headerData(index.column(), Qt::Horizontal, modelutil::CANCEL_SORTING_ROLE);
    if (!cancelColumnsData.isNull())
    {
        QVector<int> columns = cancelColumnsData.value<QVector<int>>();
        if (columns.indexOf(sortColumn()) != -1)
            sortCancelled = true;
    }

    return QSortFilterProxyModel::setData(index, value, role);
}

void ModsSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    sortCancelled = false;
    QSortFilterProxyModel::sort(column, order);
}

bool ModsSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, filterStatusColumn, sourceParent);
    modelutil::Status rowStatus = (requiredStatuses || maskedStatuses) ? sourceModel()->data(index, modelutil::STATUS_ROLE).value<Status>() : modelutil::NO_STATUS;
    if ((requiredStatuses && (rowStatus & requiredStatuses) != requiredStatuses)
        || (maskedStatuses && (rowStatus & maskedStatuses)))
        return false;

    if (filterTextColumns.isEmpty())
        return true;

    for (int column : filterTextColumns)
    {
        index = sourceModel()->index(sourceRow, column, sourceParent);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        // Pre-QT6: setFilterFixedString sets RegExp.
        if (sourceModel()->data(index).toString().contains(filterRegExp()))
            return true;
#else
        // QT6: RegExp retired. setFilterFixedString sets RegularExpression.
        if (sourceModel()->data(index).toString().contains(filterRegularExpression()))
            return true;
#endif
    }
    return false;
}

bool ModsSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (sortCancelled)
        return false;

    if (left.column() != right.column())
        return QSortFilterProxyModel::lessThan(left, right);
    int sortType = sourceModel()->headerData(left.column(), Qt::Horizontal, modelutil::SORT_ROLE).toInt();
    switch(sortType)
    {
    case modelutil::VERSION_SORT:
    case modelutil::VERSION_TIME_SORT:
        // Continue processing below.
        break;
    case modelutil::ROLE_SORT:
        return sourceModel()->data(left, modelutil::SORT_ROLE).toInt() < sourceModel()->data(right, modelutil::SORT_ROLE).toInt();
    case modelutil::MOD_ID_SORT:
        return ColumnLessThan::modId(sourceModel()->data(left), sourceModel()->data(right));
    default:
        // Skip the extraction if we're falling through to superclass method.
        return QSortFilterProxyModel::lessThan(left, right);
    }

    const QVariant leftData = sourceModel()->data(left);
    const QVariant rightData = sourceModel()->data(right);
    const QVariant leftStatusData = sourceModel()->data(left, modelutil::STATUS_ROLE);
    const QVariant rightStatusData = sourceModel()->data(right, modelutil::STATUS_ROLE);
    if (!leftStatusData.isValid() || !rightStatusData.isValid())
        return QSortFilterProxyModel::lessThan(left, right);

    const Status leftStatus = leftStatusData.value<Status>();
    const Status rightStatus = rightStatusData.value<Status>();

    switch (sortType)
    {
    case modelutil::VERSION_SORT:
        return ColumnLessThan::modVersion(leftData, leftStatus, rightData, rightStatus);
    case modelutil::VERSION_TIME_SORT:
        return ColumnLessThan::modVersionTime(leftData, leftStatus, rightData, rightStatus);
    default:
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

} // namespace iimodmanager
