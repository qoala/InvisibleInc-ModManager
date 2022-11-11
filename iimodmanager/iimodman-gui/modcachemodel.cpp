#include "modcachemodel.h"

#include <QDateTime>
#include <modinfo.h>

namespace iimodmanager {

namespace ColumnData {
    QVariant modName(const CachedMod &mod, int role)
    {
        if (role == Qt::DisplayRole)
            return mod.info().name();
        else
            return QVariant();
    }

    QVariant modId(const CachedMod &mod, int role)
    {
        if (role == Qt::DisplayRole)
            return mod.id();
        else
            return QVariant();
    }

    QVariant modLatestVersion(const CachedMod &mod, int role)
    {
        const CachedVersion *cv = mod.latestVersion();
        if (!cv)
        {
            if (role == Qt::DisplayRole)
                return QStringLiteral("N/A");
            else if (role == Qt::ToolTipRole)
                return QStringLiteral("(needs download)");
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
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return *version;
        else
            return QVariant();
    }

    QVariant modLatestUpdateTime(const CachedMod &mod, int role)
    {
        const CachedVersion *cv = mod.latestVersion();
        if (!cv)
            return QVariant();

        const std::optional<QDateTime> timestamp = cv->timestamp();
        if (!timestamp)
        {
            if (role == Qt::DisplayRole)
                return cv->id();
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return *timestamp;
        else
            return QVariant();
    }
}

ModCacheModel::ModCacheModel(ModCache &cache, QObject *parent)
    : QAbstractListModel(parent), cache(cache)
{
    connect(&cache, &ModCache::aboutToAppendMods, this, &ModCacheModel::sourceAboutToAppendMods);
    connect(&cache, &ModCache::appendedMods, this, &ModCacheModel::sourceAppendedMods);
    connect(&cache, &ModCache::aboutToRefresh, this, &ModCacheModel::sourceAboutToRefresh);
    connect(&cache, &ModCache::refreshed, this, &ModCacheModel::sourceRefreshed);
    connect(&cache, &ModCache::metadataChanged, this, &ModCacheModel::sourceMetadataChanged);
}

int ModCacheModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return cache.mods().size();
}

int ModCacheModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

QVariant ModCacheModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= cache.mods().size())
        return QVariant();

    const CachedMod &cm = cache.mods().at(index.row());

    switch (index.column())
    {
        case NAME:
            return ColumnData::modName(cm, role);
        case ID:
            return ColumnData::modId(cm, role);
        case LATEST_VERSION:
            return ColumnData::modLatestVersion(cm, role);
        case UPDATE_TIME:
            return ColumnData::modLatestUpdateTime(cm, role);
        default:
            return QVariant();
    }
}

QVariant ModCacheModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
            case NAME:
                return QStringLiteral("Name");
            case ID:
                return QStringLiteral("ID");
            case LATEST_VERSION:
                return QStringLiteral("Latest Version");
            case UPDATE_TIME:
                return QStringLiteral("Update Time");
            default:
                return QVariant();
        }
    }
    else
        return QVariant();
}

void ModCacheModel::sourceAboutToAppendMods(const QStringList &modIds)
{
    int start = cache.mods().size();
    beginInsertRows(QModelIndex(), start, start + modIds.size());
}

void ModCacheModel::sourceAppendedMods()
{
    endInsertRows();
}

void ModCacheModel::sourceAboutToRefresh(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint)
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

void ModCacheModel::sourceRefreshed(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint)
{
    if (hint == ModCache::VERSION_ONLY_HINT)
    {
        // Changed metadata only.
        sourceMetadataChanged(modIds, modIdxs);
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

void ModCacheModel::sourceMetadataChanged(const QStringList &modIds, const QList<int> &modIdxs)
{
    Q_UNUSED(modIds);

    // TODO: For modIdxs size > 1, check if consecutive, then emit a properly bounded change event.
    if (modIdxs.size() == 1)
        // One row changed.
        emit dataChanged(createIndex(modIdxs[0], COLUMN_MIN), createIndex(modIdxs[0], COLUMN_MAX));
    else
        // Any/All rows changed.
        emit dataChanged(createIndex(0, COLUMN_MIN), createIndex(cache.mods().size()-1, COLUMN_MAX));
}

}  // namespace iimodmanager
