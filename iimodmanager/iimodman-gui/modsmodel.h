#ifndef MODSMODEL_H
#define MODSMODEL_H

#include <QAbstractListModel>
#include <modcache.h>

namespace iimodmanager {

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
        NAME = 0,
        ID = 1,
        INSTALLED_VERSION = 2,
        INSTALLED_UPDATE_TIME = 3,
        LATEST_VERSION = 4,
        CACHE_UPDATE_TIME = 5,

        COLUMN_MIN = 0,
        COLUMN_MAX = CACHE_UPDATE_TIME,
        COLUMN_COUNT = COLUMN_MAX+1,
    };
    enum UserDataRole
    {
        STATUS_ROLE = 0x100,
    };
    enum StatusFlag
    {
        NO_STATUS = 0,
        //! The row is a mod that's installed.
        INSTALLED_STATUS = 1,
        //! The row is a mod that's in the cache.
        CACHED_STATUS = 2,
        //! The row is a steam workshop mod.
        STEAM_STATUS = 4,

        //! The row is a mod that's not downloaded.
        NO_DOWNLOAD_STATUS = 0x10,
        //! The row is a mod that's not downloaded.
        CAN_DOWNLOAD_UPDATE_STATUS = 0x20,
        //! The row is a mod that's not downloaded.
        CAN_INSTALL_UPDATE_STATUS = 0x40,

        //! The requested column is null for the requested row.
        //! Example: If a given mod version does not exist, it is NULL.
        NULL_STATUS = 0x100,
        //! The requested column is unlabelled for the requested mod.
        //! Example: If a given mod version exists, but does not declare a 'version' string, it is UNLABELLED.
        UNLABELLED_STATUS = 0x200,
    };
    Q_DECLARE_FLAGS(Status, StatusFlag)
    Q_FLAG(Status)

    ModsModel(ModCache &cache, ModList &modList, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

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

    ModCache &cache;
    ModList &modList;
    // Persistent tracking of mods in the installed, but not download-cached lists.
    QVector<int> uncachedIdxs_;
    QHash<QString, int> uncachedIds_; // Mod ID to index in uncachedIdxs.

    // Temporary storage between aboutTo and completion signals.
    AppendType appendType;
    QModelIndexList savedPersistentIndexes;
    QVector<QString> savedPersistentMappings;

    inline const QVector<int> &uncachedIdxs() const { return uncachedIdxs_; };
    //! Returns the row number for an uncached mod ID, or -1 if not found.
    //! \sa ModCache::modIndex
    int uncachedIndex(const QString &modId) const;

    void reindexUncachedMods();
    void savePersistentIndexes();
    void updatePersistentIndexes();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ModsModel::Status)

}  // namespace iimodmanager

#endif // MODSMODEL_H
