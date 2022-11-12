#include "modsmodel.h"

#include <QDateTime>
#include <modcache.h>
#include <modinfo.h>

namespace iimodmanager {

namespace ColumnData {
    typedef ModsModel::Status Status;

    //! Returns row-level status flags.
    Status modStatus(const CachedMod *cm)
    {
        ModsModel::Status status;
        if (!cm->downloaded())
            status |= ModsModel::NO_DOWNLOAD_STATUS;
        // TODO: Store recent SteamModInfo in ModCache and track CAN_DOWNLOAD_UPDATE_STATUS.

        if (cm->installedVersion())
        {
            status |= ModsModel::INSTALLED_STATUS;
            if (cm->installedVersion() != cm->latestVersion())
                status |= ModsModel::CAN_INSTALL_UPDATE_STATUS;
        }

        return status;
    }

    QVariant modName(const CachedMod *cm, int role)
    {
        if (role == Qt::DisplayRole)
            return cm->info().name();
        else if (role == ModsModel::STATUS_ROLE)
            return QVariant::fromValue<Status>(modStatus(cm));
        else
            return QVariant();
    }

    QVariant modId(const CachedMod *cm, int role)
    {
        if (role == Qt::DisplayRole)
            return cm->id();
        else if (role == ModsModel::STATUS_ROLE)
            return QVariant::fromValue<Status>(modStatus(cm));
        else
            return QVariant();
    }

    QVariant cacheLatestVersion(const CachedMod *cm, int role)
    {
        const CachedVersion *cv = cm->latestVersion();
        if (!cv)
        {
            if (role == Qt::DisplayRole)
                return QStringLiteral("N/A");
            else if (role == Qt::ToolTipRole)
                return QStringLiteral("(needs download)");
            else if (role == ModsModel::STATUS_ROLE)
                // NO_DOWNLOAD is covered by the mod status flags.
                return QVariant::fromValue<Status>(modStatus(cm));
            else
                return QVariant();
        }

        const std::optional<QString> version = cv->version();
        if (!version)
        {
            if (role == Qt::DisplayRole)
                return QStringLiteral("-");
            else if (role == Qt::ToolTipRole)
                return QStringLiteral("(unlabeled)");
            else if (role == ModsModel::STATUS_ROLE)
                return QVariant::fromValue<Status>(modStatus(cm) | ModsModel::UNLABELLED_STATUS);
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return *version;
        else if (role == ModsModel::STATUS_ROLE)
            return QVariant::fromValue<Status>(modStatus(cm));
        else
            return QVariant();
    }

    QVariant cacheUpdateTime(const CachedMod *cm, int role)
    {
        const CachedVersion *cv = cm->latestVersion();
        if (!cv)
        {
            if (role == ModsModel::STATUS_ROLE)
                return QVariant::fromValue<Status>(modStatus(cm));
            else
                return QVariant();
        }

        const std::optional<QDateTime> timestamp = cv->timestamp();
        if (!timestamp)
        {
            if (role == Qt::DisplayRole)
                return cv->id();
            else if (role == ModsModel::STATUS_ROLE)
                return QVariant::fromValue<Status>(modStatus(cm) | ModsModel::UNLABELLED_STATUS);
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return *timestamp;
        else if (role == ModsModel::STATUS_ROLE)
            return QVariant::fromValue<Status>(modStatus(cm));
        else
            return QVariant();
    }
}

ModsModel::ModsModel(ModCache &cache, QObject *parent)
    : QAbstractListModel(parent), cache(cache)
{
    connect(&cache, &ModCache::aboutToAppendMods, this, &ModsModel::cacheAboutToAppendMods);
    connect(&cache, &ModCache::appendedMods, this, &ModsModel::cacheAppendedMods);
    connect(&cache, &ModCache::aboutToRefresh, this, &ModsModel::cacheAboutToRefresh);
    connect(&cache, &ModCache::refreshed, this, &ModsModel::cacheRefreshed);
    connect(&cache, &ModCache::metadataChanged, this, &ModsModel::cacheMetadataChanged);
}

int ModsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return cache.mods().size();
}

int ModsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

QVariant ModsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= cache.mods().size())
        return QVariant();

    const CachedMod *cm = &cache.mods().at(index.row());

    switch (index.column())
    {
        case NAME:
            return ColumnData::modName(cm, role);
        case ID:
            return ColumnData::modId(cm, role);
        case LATEST_VERSION:
            return ColumnData::cacheLatestVersion(cm, role);
        case CACHE_UPDATE_TIME:
            return ColumnData::cacheUpdateTime(cm, role);
        default:
            return QVariant();
    }
}

QVariant ModsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role == Qt::DisplayRole)
        switch (section)
        {
            case NAME:
                return QStringLiteral("Name");
            case ID:
                return QStringLiteral("ID");
            case LATEST_VERSION:
                return QStringLiteral("Latest Version");
            case CACHE_UPDATE_TIME:
                return QStringLiteral("Update Time");
        }
    else if (role == Qt::InitialSortOrderRole)
        switch (section)
        {
            case NAME:
                return Qt::AscendingOrder;
            case ID:
                return Qt::AscendingOrder;
            case LATEST_VERSION:
                return Qt::DescendingOrder;
            case CACHE_UPDATE_TIME:
                return Qt::DescendingOrder;
        }
    // TODO: Qt::BackgroundRole with different backgrounds based on mod status.

    return QVariant();
}

void ModsModel::cacheAboutToAppendMods(const QStringList &modIds)
{
    int start = cache.mods().size();
    beginInsertRows(QModelIndex(), start, start + modIds.size());
}

void ModsModel::cacheAppendedMods()
{
    endInsertRows();
}

void ModsModel::cacheAboutToRefresh(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint)
{
    Q_UNUSED(modIds);
    Q_UNUSED(modIdxs);

    if (hint == ModCache::VERSION_ONLY_HINT)
        // Changing metadata only. Will emit dataChanged instead.
        return;

    if (hint == ModCache::SORT_ONLY_HINT)
        emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
    else
        emit layoutAboutToBeChanged();

    savedPersistentIndexes.clear();
    savedPersistentMappings.clear();
    savedPersistentMappings.resize(cache.mods().size());

    for (const QModelIndex &index : savedPersistentIndexes)
    {
        if (savedPersistentMappings[index.row()].isNull())
            savedPersistentMappings[index.row()] = cache.mods().at(index.row()).id();
    }
}

void ModsModel::cacheRefreshed(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint)
{
    if (hint == ModCache::VERSION_ONLY_HINT)
    {
        // Changed metadata only.
        cacheMetadataChanged(modIds, modIdxs);
        return;
    }

    QModelIndexList to;
    to.reserve(savedPersistentIndexes.size());
    for (const auto &fromIndex : savedPersistentIndexes)
    {
        const QString &modId = savedPersistentMappings.at(fromIndex.row());
        int newIdx = cache.modIndex(modId);
        if (newIdx == -1)
            to << createIndex(newIdx, fromIndex.column());
        else
            to << QModelIndex(); // Mod is no longer visible.
    }
    changePersistentIndexList(savedPersistentIndexes, to);

    savedPersistentIndexes.clear();
    savedPersistentMappings.clear();

    if (hint == ModCache::SORT_ONLY_HINT)
        emit layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
    else
        emit layoutChanged();
}

void ModsModel::cacheMetadataChanged(const QStringList &modIds, const QList<int> &modIdxs)
{
    Q_UNUSED(modIds);

    if (modIdxs.size() == 0)
        // Any/All rows changed.
        emit dataChanged(createIndex(0, COLUMN_MIN), createIndex(cache.mods().size()-1, COLUMN_MAX));
    else if (modIdxs.size() == 1)
        // One row changed.
        emit dataChanged(createIndex(modIdxs[0], COLUMN_MIN), createIndex(modIdxs[0], COLUMN_MAX));
    else
    {
        // Multiple rows changed. Declare a bounding rectangle.
        int minRow = modIdxs[0];
        int maxRow = minRow;
        for (int idx : modIdxs)
        {
            if (idx < minRow)
                minRow = idx;
            else if (idx > maxRow)
                maxRow = idx;
        }
        emit dataChanged(createIndex(minRow, COLUMN_MIN), createIndex(maxRow, COLUMN_MAX));
    }
}

}  // namespace iimodmanager
