#include "modsmodel.h"
#include "modssortfilterproxymodel.h"

#include <QDateTime>
#include <QRegularExpression>

namespace iimodmanager {

typedef ModsModel::Status Status;

namespace ColumnLessThan {
    bool modVersion(const QVariant &leftData, Status leftStatus, const QVariant &rightData, Status rightStatus)
    {
        // NULL is less than any other value.
        if (leftStatus.testFlag(ModsModel::NULL_STATUS) || leftStatus.testFlag(ModsModel::NULL_STATUS))
            return leftStatus.testFlag(ModsModel::NULL_STATUS) && !rightStatus.testFlag(ModsModel::NULL_STATUS);
        // UNLABELLED is less than any provided value.
        if (leftStatus.testFlag(ModsModel::UNLABELLED_STATUS) || rightStatus.testFlag(ModsModel::UNLABELLED_STATUS))
            return leftStatus.testFlag(ModsModel::UNLABELLED_STATUS) && !rightStatus.testFlag(ModsModel::UNLABELLED_STATUS);

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

    bool modUpdateTime(const QVariant &leftData, Status leftStatus, const QVariant &rightData, Status rightStatus)
    {
        // NULL is less than any other value.
        if (leftStatus.testFlag(ModsModel::NULL_STATUS) || leftStatus.testFlag(ModsModel::NULL_STATUS))
            return leftStatus.testFlag(ModsModel::NULL_STATUS) && !rightStatus.testFlag(ModsModel::NULL_STATUS);

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
    : QSortFilterProxyModel(parent) {}

void ModsSortFilterProxyModel::setFilterStatus(ModsModel::Status requiredStatuses, ModsModel::Status maskedStatuses)
{
    this->requiredStatuses = requiredStatuses;
    this->maskedStatuses = maskedStatuses;
    invalidateFilter();
}

bool ModsSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex nameIndex = sourceModel()->index(sourceRow, ModsModel::NAME, sourceParent);
    QModelIndex modIdIndex = sourceModel()->index(sourceRow, ModsModel::ID, sourceParent);
    ModsModel::Status rowStatus = (requiredStatuses || maskedStatuses) ? sourceModel()->data(nameIndex, ModsModel::STATUS_ROLE).value<Status>() : ModsModel::NO_STATUS;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Pre-QT6: setFilterFixedString sets RegExp.
    return (sourceModel()->data(nameIndex).toString().contains(filterRegExp())
            || sourceModel()->data(modIdIndex).toString().contains(filterRegExp()))
#else
    // QT6: RegExp retired. setFilterFixedString sets RegularExpression.
    return (sourceModel()->data(nameIndex).toString().contains(filterRegularExpression())
            || sourceModel()->data(modIdIndex).toString().contains(filterRegularExpression()))
#endif
        && (!requiredStatuses || (rowStatus & requiredStatuses) == requiredStatuses)
        && (!maskedStatuses || !(rowStatus & maskedStatuses));
}

bool ModsSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.column() != right.column())
        return QSortFilterProxyModel::lessThan(left, right);
    switch(left.column())
    {
    case ModsModel::INSTALLED_VERSION:
    case ModsModel::INSTALLED_UPDATE_TIME:
    case ModsModel::LATEST_VERSION:
    case ModsModel::CACHE_UPDATE_TIME:
        break;
    default:
        // Skip the extraction if we're falling through to superclass method.
        return QSortFilterProxyModel::lessThan(left, right);
    }

    const QVariant leftData = sourceModel()->data(left);
    const QVariant rightData = sourceModel()->data(right);
    const QVariant leftStatusData = sourceModel()->data(left, ModsModel::STATUS_ROLE);
    const QVariant rightStatusData = sourceModel()->data(right, ModsModel::STATUS_ROLE);
    if (!leftStatusData.isValid() || !rightStatusData.isValid())
        return QSortFilterProxyModel::lessThan(left, right);

    const Status leftStatus = leftStatusData.value<Status>();
    const Status rightStatus = rightStatusData.value<Status>();

    switch (left.column())
    {
    case ModsModel::INSTALLED_VERSION:
    case ModsModel::LATEST_VERSION:
        return ColumnLessThan::modVersion(leftData, leftStatus, rightData, rightStatus);
    case ModsModel::INSTALLED_UPDATE_TIME:
    case ModsModel::CACHE_UPDATE_TIME:
        return ColumnLessThan::modUpdateTime(leftData, leftStatus, rightData, rightStatus);
    default:
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

} // namespace iimodmanager
