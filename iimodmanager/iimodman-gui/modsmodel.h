#ifndef MODSMODEL_H
#define MODSMODEL_H

#include "modelutil.h"

#include <QAbstractListModel>
#include <modcache.h>

namespace iimodmanager {

class ModCache;
class ModList;

//! Data model representing the union of the download-cached and installed mods.
//! Changes to the download cache are reflected immediately.
//! Changes to the installed mods require a refresh of the mod list.
class ModsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Column
    {
        NAME,
        ID,
        INSTALLED_VERSION,
        INSTALLED_VERSION_TIME,
        LATEST_VERSION,
        CACHE_UPDATE_TIME,

        COLUMN_MAX = CACHE_UPDATE_TIME,
        COLUMN_COUNT = COLUMN_MAX+1,
    };

    ModsModel(const ModCache &cache, const ModList &modList, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

signals:
    void textOutput(QString value);

protected:
    const ModCache &cache;
    const ModList &modList;

    inline int columnMin() const { return 0; };
    //! The highest column number accepted by this model. ('columnCount() - 1' if columnCount is constant).
    //! Subclasses may alter the visible columns. Override columnMax and columnCount as necessary.
    virtual int columnMax() const;

    //! Called after ModsModel has processed a change that only affects cached mods.
    //! Subclass implementations must call the base method(or the callback),
    //! which will emit the appropriate upward signal.
    virtual void reportCacheChanged(const std::function<void ()> &cb);
    //! Called after ModsModel has processed a change that may affect both cached and installed mods.
    //! Subclass implementations must call the base method (or the callback),
    //! which will emit the appropriate upward signal.
    virtual void reportAllChanged(const std::function<void ()> &cb);

    //! Returns the cached and downloaded mod resources for the given row into its arguments.
    void seekRow(int row, const CachedMod **cmOut, const InstalledMod **imOut) const;
    inline const QVector<int> &uncachedIdxs() const { return uncachedIdxs_; };
    //! Returns the row number for an uncached mod ID, or -1 if not found.
    //! \sa ModCache::modIndex
    int uncachedIndex(const QString &modId) const;

    void reindexUncachedMods();
    void savePersistentIndexes();
    void updatePersistentIndexes();

private slots:
    void cacheAboutToAppendMods(const QStringList &modIds);
    void cacheAppendedMods();
    void cacheAboutToRefresh(const QStringList &modIds = QStringList(), const QList<int> &modIdxs = QList<int>(), ModCache::ChangeHint hint = ModCache::NO_HINT);
    void cacheRefreshed(const QStringList &modIds = QStringList(), const QList<int> &modIdxs = QList<int>(), ModCache::ChangeHint hint = ModCache::NO_HINT);
    void cacheMetadataChanged(const QStringList &modIds = QStringList(), const QList<int> &modIdxs = QList<int>());
    void installedModsAboutToRefresh();
    void installedModsRefreshed();

private:
    enum AppendType
    {
        //! The current cache-append event is adding zero rows.
        EMPTY_APPEND,
        //! The current cache-append event is adding new mods to the cache.
        NORMAL_APPEND,
        //! The current cache-append event is transferring installed-only mods to the cache.
        MOVE_APPEND,
        //! The current cache-append event is transferring installed-only mods to the cache without reordering the visible rows.
        TRIVIAL_MOVE_APPEND,
        //! The current cache-append event has a non-simple intersection with installed-only mods.
        //! Treated as a full refresh.
        REFRESH_APPEND,
    };

    // Persistent tracking of mods in the installed, but not download-cached lists.
    QVector<int> uncachedIdxs_;
    QHash<QString, int> uncachedIds_; // Mod ID to index in uncachedIdxs.

    // Temporary storage between aboutTo and completion signals.
    AppendType appendType;
    QModelIndexList savedPersistentIndexes;
    QVector<QString> savedPersistentMappings;
};

}  // namespace iimodmanager

#endif // MODSMODEL_H
