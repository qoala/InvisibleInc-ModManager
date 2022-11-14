#include "modelutil.h"
#include "modsmodel.h"

#include <QDateTime>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

namespace ColumnData {
    typedef modelutil::Status Status;

    QVariant modName(const CachedMod *cm, const InstalledMod *im, int role)
    {
        if (role == Qt::DisplayRole)
        {
            if (cm)
                return cm->info().name();
            else if (im)
                return im->info().name();
        }
        else if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(modelutil::modStatus(cm, im));
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
        else if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(modelutil::modStatus(cm, im));
        return QVariant();
    }

    QVariant installedLatestVersion(const CachedMod *cm, const InstalledMod *im, int role)
    {
        Status status = modelutil::modStatus(cm, im, role);

        if (!im)
        {
            if (role == modelutil::STATUS_ROLE)
                return modelutil::toVariant(status | modelutil::NULL_STATUS);
            return QVariant();
        }

        const QString &version = im->info().version();
        return modelutil::versionData(version, status, role);
    }

    QVariant cacheLatestVersion(const CachedMod *cm, int role)
    {
        Status status = modelutil::modStatus(cm, nullptr, role);

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
            else if (role == modelutil::STATUS_ROLE)
                return modelutil::toVariant(status | modelutil::NULL_STATUS);
            return QVariant();
        }

        const QString version = cv->version().value_or(QString());
        return modelutil::versionData(version, status, role);
    }

    QVariant installedUpdateTime(const CachedMod *cm, const InstalledMod *im, int role)
    {
        Status status = modelutil::modStatus(cm, im, role);

        if (!im)
        {
            if (role == modelutil::STATUS_ROLE)
                return modelutil::toVariant(status | modelutil::NULL_STATUS);
            else
                return QVariant();
        }

        const CachedVersion *cv = im->cacheVersion();
        if (!cv && role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(status | modelutil::UNLABELLED_STATUS);

        return modelutil::versionTimeData(cv, status, role);
    }

    QVariant cacheUpdateTime(const CachedMod *cm, int role)
    {
        Status status = modelutil::modStatus(cm, nullptr, role);

        const CachedVersion *cv = cm ? cm->latestVersion() : nullptr;
        if (!cv && role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(status | modelutil::NULL_STATUS);
        return modelutil::versionTimeData(cv, status, role);
    }
}

ModsModel::ModsModel(const ModCache &cache, const ModList &modList, QObject *parent)
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

int ModsModel::columnMax() const
{
    return COLUMN_MAX;
}

void ModsModel::seekRow(int row, const CachedMod **cm, const InstalledMod **im) const
{
    if (!cm || !im)
        return;

    int cacheSize = cache.mods().size();
    *cm = nullptr;
    *im = nullptr;
    if (row < 0)
        return;
    else if (row < cacheSize)
    {
        *cm = &cache.mods().at(row);
        *im = *cm ? modList.mod((*cm)->id()) : nullptr;
    }
    else if ((row -= cacheSize) < uncachedIdxs().size())
    {
        *im = &modList.mods().at(uncachedIdxs().at(row));
    }
}

int ModsModel::rowOf(const QString &modId) const
{
    int row = cache.modIndex(modId);
    if (row != -1)
        return row;
    return uncachedIndex(modId);
}

QVariant ModsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const CachedMod *cm;
    const InstalledMod *im;
    seekRow(index.row(), &cm, &im);
    if (!cm && !im)
        return QVariant();

    switch (index.column())
    {
    case NAME:
        return ColumnData::modName(cm, im, role);
    case ID:
        return ColumnData::modId(cm, im, role);
    case INSTALLED_VERSION:
        return ColumnData::installedLatestVersion(cm, im, role);
    case INSTALLED_VERSION_TIME:
        return ColumnData::installedUpdateTime(cm, im, role);
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
        case INSTALLED_VERSION:
            return QStringLiteral("Installed Version");
        case INSTALLED_VERSION_TIME:
            return QStringLiteral("Installed Version Time");
        case LATEST_VERSION:
            return QStringLiteral("Latest Version");
        case CACHE_UPDATE_TIME:
            return QStringLiteral("Latest Version Time");
        }
    else if (role == Qt::InitialSortOrderRole)
        switch (section)
        {
        case NAME:
        case ID:
            return Qt::AscendingOrder;
        case INSTALLED_VERSION:
        case LATEST_VERSION:
        case INSTALLED_VERSION_TIME:
        case CACHE_UPDATE_TIME:
            return Qt::DescendingOrder;
        }
    else if (role == modelutil::SORT_ROLE)
        switch (section)
        {
        case NAME:
            return modelutil::NORMAL_SORT;
        case ID:
            return modelutil::MOD_ID_SORT;
        case INSTALLED_VERSION:
        case LATEST_VERSION:
            return modelutil::VERSION_SORT;
        case INSTALLED_VERSION_TIME:
        case CACHE_UPDATE_TIME:
            return modelutil::VERSION_TIME_SORT;
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
        int newIdx = rowOf(modId);
        if (newIdx != -1)
            to << createIndex(newIdx, fromIndex.column());
        else
            to << QModelIndex(); // Mod is no longer available.
    }
    changePersistentIndexList(savedPersistentIndexes, to);

    savedPersistentIndexes.clear();
    savedPersistentMappings.clear();
}

void ModsModel::reportCacheChanged(const std::function<void ()> &cb, const QString &modId)
{
    cb();
}

void ModsModel::reportAllChanged(const std::function<void ()> &cb, const QString &modId)
{
    cb();
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
    switch (appendType)
    {
    EMPTY_APPEND:
        return;
    NORMAL_APPEND:
        reportCacheChanged([this]() { endInsertRows(); });
        return;
    TRIVIAL_MOVE_APPEND:
        reindexUncachedMods();
        {
            const int row = cache.mods().size() - 1;
            const int min = columnMin();
            const int max = columnMax();
            reportAllChanged([this, row, min, max]()
                    {
                        emit dataChanged(
                                createIndex(row, min),
                                createIndex(row, max));
                    });
        }
        return;
    MOVE_APPEND:
        reindexUncachedMods();
        {
            const int row = cache.mods().size() - 1;
            const int min = columnMin();
            const int max = columnMax();
            reportAllChanged([this, row, min, max]()
                    {
                        endMoveRows();
                        emit dataChanged(
                                createIndex(row, min),
                                createIndex(row, max));
                    });
        }
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
        reportAllChanged([this]()
                {
                    emit layoutChanged(
                            QList<QPersistentModelIndex>(),
                            QAbstractItemModel::VerticalSortHint);
                });
    else
        reportAllChanged([this]() { layoutChanged(); });
}

void ModsModel::cacheMetadataChanged(const QStringList &modIds, const QList<int> &modIdxs)
{
    int startRow, endRow;
    const int startColumn = columnMin(), endColumn = columnMax();
    QString modId;
    if (modIdxs.size() == 0)
    {
        // Any/All cache rows changed.
        startRow = 0;
        endRow = cache.mods().size()-1;
    }
    else if (modIdxs.size() == 1)
    {
        // One row changed.
        startRow = modIdxs[0];
        endRow = modIdxs[0];
        modId = modIds[0];
    }
    else
    {
        // Multiple rows changed. Declare a bounding rectangle.
        startRow = modIdxs[0];
        endRow = startRow;
        for (int idx : modIdxs)
        {
            if (idx < startRow)
                startRow = idx;
            else if (idx > endRow)
                endRow = idx;
        }
    }
    reportAllChanged([this, startRow, startColumn, endRow, endColumn]()
            {
                emit dataChanged(
                        createIndex(startRow, startColumn),
                        createIndex(endRow, endColumn));
            }, modId);
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

    reportAllChanged([this]() { emit layoutChanged(); });
}

}  // namespace iimodmanager
