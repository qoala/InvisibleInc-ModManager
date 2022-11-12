#include "modsmodel.h"

#include <QDateTime>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

namespace ColumnData {
    typedef ModsModel::Status Status;

    //! Returns row-level status flags.
    Status modStatus(const CachedMod *cm, const InstalledMod *im = nullptr)
    {
        ModsModel::Status status;
        if (cm)
        {
            status |= ModsModel::CACHED_STATUS;
            if (cm->info().isSteam())
                status |= ModsModel::STEAM_STATUS;
            if (!cm->downloaded())
                status |= ModsModel::NO_DOWNLOAD_STATUS;
            // TODO: Store recent SteamModInfo in ModCache and track CAN_DOWNLOAD_UPDATE_STATUS.

            if (cm->installedVersion())
            {
                status |= ModsModel::INSTALLED_STATUS;
                if (cm->installedVersion() != cm->latestVersion())
                    status |= ModsModel::CAN_INSTALL_UPDATE_STATUS;
            }
        }
        else if (im) // Should always be present if no cached mod.
        {
            // Installed, but uncached.
            status |= ModsModel::INSTALLED_STATUS;
            if (im->info().isSteam())
                status |= ModsModel::STEAM_STATUS;
        }

        return status;
    }

    QVariant modName(const CachedMod *cm, const InstalledMod *im, int role)
    {
        if (role == Qt::DisplayRole)
        {
            if (cm)
                return cm->info().name();
            else if (im)
                return im->info().name();
        }
        else if (role == ModsModel::STATUS_ROLE)
            return QVariant::fromValue<Status>(modStatus(cm, im));
        return QVariant();
    }

    QVariant modId(const CachedMod *cm, const InstalledMod *im, int role)
    {
        if (role == Qt::DisplayRole)
        {
            if (cm)
                return cm->id();
            else if (im)
                return im->id();
        }
        else if (role == ModsModel::STATUS_ROLE)
            return QVariant::fromValue<Status>(modStatus(cm, im));
        return QVariant();
    }

    QVariant cacheLatestVersion(const CachedMod *cm, int role)
    {
        const CachedVersion *cv = cm ? cm->latestVersion() : nullptr;
        if (!cv)
        {
            if (role == Qt::DisplayRole)
                return QStringLiteral("N/A");
            else if (role == Qt::ToolTipRole)
                if (cm)
                    return QStringLiteral("(needs download)");
                else
                    return QStringLiteral("(needs copy installed to cache)");
            else if (role == ModsModel::STATUS_ROLE)
                // NO_DOWNLOAD is covered by the mod status flags.
                return QVariant::fromValue<Status>(modStatus(cm));
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
        const CachedVersion *cv = cm ? cm->latestVersion() : nullptr;
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

ModsModel::ModsModel(ModCache &cache, ModList &modList, QObject *parent)
    : QAbstractListModel(parent), cache(cache), modList(modList)
{
    reindexUncachedMods();
    connect(&cache, &ModCache::aboutToAppendMods, this, &ModsModel::cacheAboutToAppendMods);
    connect(&cache, &ModCache::appendedMods, this, &ModsModel::cacheAppendedMods);
    connect(&cache, &ModCache::aboutToRefresh, this, &ModsModel::cacheAboutToRefresh);
    connect(&cache, &ModCache::refreshed, this, &ModsModel::cacheRefreshed);
    connect(&cache, &ModCache::metadataChanged, this, &ModsModel::cacheMetadataChanged);
    connect(&modList, &ModList::aboutToRefresh, this, &ModsModel::installedModsAboutToRefresh);
    connect(&modList, &ModList::refreshed, this, &ModsModel::installedModsRefreshed);
}

int ModsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return cache.mods().size() + uncachedIdxs().size();
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

    int row = index.row();
    int cacheSize = cache.mods().size();
    const CachedMod *cm = nullptr;
    const InstalledMod *im = nullptr;
    if (row < 0)
        return QVariant();
    else if (row < cacheSize)
    {
        cm = &cache.mods().at(row);
        im = modList.mod(cm->id());
    }
    else if ((row -= cacheSize) < uncachedIdxs().size())
    {
        im = &modList.mods().at(uncachedIdxs().at(row));
    }
    if (!cm && !im)
        return QVariant();

    switch (index.column())
    {
    case NAME:
        return ColumnData::modName(cm, im, role);
    case ID:
        return ColumnData::modId(cm, im, role);
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

int ModsModel::uncachedIndex(const QString &modId) const
{
    if (uncachedIds_.contains(modId))
        return uncachedIds_[modId] + cache.mods().size();
    return -1;
}

void ModsModel::reindexUncachedMods()
{
    uncachedIdxs_.clear();
    uncachedIds_.clear();

    for (int i = 0; i < modList.mods().size(); ++i)
    {
        const QString &modId = modList.mods().at(i).id();
        if (!cache.contains(modId))
        {
            uncachedIds_.insert(modId, uncachedIdxs_.size());
            uncachedIdxs_.append(i);
        }
    }
}

void ModsModel::savePersistentIndexes()
{
    savedPersistentIndexes = persistentIndexList();
    savedPersistentMappings.clear();
    savedPersistentMappings.resize(cache.mods().size() + uncachedIdxs().size());

    for (const QModelIndex &index : savedPersistentIndexes)
    {
        if (savedPersistentMappings[index.row()].isNull())
        {
            int row = index.row();
            int cacheSize = cache.mods().size();
            if (row < cacheSize)
                savedPersistentMappings[row] = cache.mods().at(row).id();
            else
                savedPersistentMappings[row] = modList.mods().at(uncachedIdxs().at(row - cacheSize)).id();
        }
    }
}

void ModsModel::updatePersistentIndexes()
{
    QModelIndexList to;
    to.reserve(savedPersistentIndexes.size());
    for (const auto &fromIndex : savedPersistentIndexes)
    {
        const QString &modId = savedPersistentMappings.at(fromIndex.row());
        int newIdx = cache.modIndex(modId);
        if (newIdx != -1)
            to << createIndex(newIdx, fromIndex.column());
        else if ((newIdx = uncachedIndex(modId)) != -1)
            to << createIndex(newIdx, fromIndex.column());
        else
            to << QModelIndex(); // Mod is no longer available.
    }
    changePersistentIndexList(savedPersistentIndexes, to);

    savedPersistentIndexes.clear();
    savedPersistentMappings.clear();
}

void ModsModel::cacheAboutToAppendMods(const QStringList &modIds)
{
    if (modIds.isEmpty())
    {
        appendType = EMPTY_APPEND;
        return;
    }

    // Check if this cache-append will modify the uncached set.
    if (!uncachedIdxs().isEmpty())
    {
        int fromRow;
        if (modIds.size() != 1)
        {
            appendType = REFRESH_APPEND;
            cacheAboutToRefresh();
            return;
        }
        else if ((fromRow = uncachedIndex(modIds[0])) == 0)
        {
            // "Moving" from the first uncached row to the last cached row. (Same row index)
            appendType = TRIVIAL_MOVE_APPEND;
            return;
        }
        else if (fromRow != 1)
        {
            appendType = MOVE_APPEND;
            int toRow = cache.mods().size();
            if (!beginMoveRows(QModelIndex(), fromRow, fromRow + 1, QModelIndex(), toRow))
            {
                // Something is wrong. Can't abort the cache change, so report a full refresh.
                appendType = REFRESH_APPEND;
                cacheAboutToRefresh();
            }
            return;
        }
    }

    appendType = NORMAL_APPEND;
    int start = cache.mods().size();
    beginInsertRows(QModelIndex(), start, start + modIds.size());
}

void ModsModel::cacheAppendedMods()
{
    int row;
    switch (appendType)
    {
    EMPTY_APPEND:
        return;
    NORMAL_APPEND:
        endInsertRows();
        return;
    TRIVIAL_MOVE_APPEND:
        reindexUncachedMods();
        row = cache.mods().size() - 1;
        emit dataChanged(createIndex(row, COLUMN_MIN), createIndex(row, COLUMN_MAX));
        return;
    MOVE_APPEND:
        reindexUncachedMods();
        endMoveRows();
        row = cache.mods().size() - 1;
        emit dataChanged(createIndex(row, COLUMN_MIN), createIndex(row, COLUMN_MAX));
        return;
    REFRESH_APPEND:
        cacheRefreshed();
        return;
    }
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

    savePersistentIndexes();
}

void ModsModel::cacheRefreshed(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint)
{
    if (hint == ModCache::VERSION_ONLY_HINT)
    {
        // Changed metadata only.
        cacheMetadataChanged(modIds, modIdxs);
        return;
    }

    reindexUncachedMods();
    updatePersistentIndexes();

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

void ModsModel::installedModsAboutToRefresh()
{
    emit layoutAboutToBeChanged();
    savePersistentIndexes();
}

void ModsModel::installedModsRefreshed()
{
    reindexUncachedMods();
    updatePersistentIndexes();

    emit layoutChanged();
}

}  // namespace iimodmanager
