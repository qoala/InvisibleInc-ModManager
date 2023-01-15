#ifndef IIMODMANAGER_MODCACHE_H
#define IIMODMANAGER_MODCACHE_H

#include "iimodman-lib_global.h"

#include <experimental/propagate_const>

#include <QLoggingCategory>
#include <QObject>
#include <memory>
#include <optional>

class QDateTime;
template <typename T> class QList;


namespace iimodmanager {

class ModInfo;
class ModManConfig;
class SpecMod;
struct SteamModInfo;


Q_DECLARE_LOGGING_CATEGORY(modcache)

class CachedMod;
class CachedVersion;


enum StringFormat {
    FORMAT_SHORT,
    FORMAT_FULL,
};

//! Mods cached locally, but not installed. May be downloaded or just have an ID-to-name registration.
class IIMODMANLIBSHARED_EXPORT ModCache : public QObject
{
    Q_OBJECT

public:
    //! Private implementation. Only accessible to classes in this file.
    class Impl;
    //! Specifies how deeply a cache refresh should check the filesystem.
    enum RefreshLevel
    {
        //! Refresh all fields on all mods and all versions.
        FULL,
        //! Refresh only available IDs on all mods and all versions.
        ID_ONLY,
        //! Refresh the latest version of each mod fully, with available IDs for older versions.
        LATEST_ONLY,
    };
    //! Specifies restrictions on what may change in a refresh event.
    enum ChangeHint
    {
        //! Anything may change.
        NO_HINT,
        //! Mods will be re-sorted. No mods will be added or removed.
        SORT_ONLY_HINT,
        //! Only metadata and versions will change. No mods will be added or removed.
        VERSION_ONLY_HINT,
    };

    ModCache(const ModManConfig &config, QObject *parent = nullptr);

    const QList<CachedMod> &mods() const;
    bool contains(const QString &id) const;
    //! The position of the mod with the given ID in the mods list, or -1 if it isn't in the cache.
    int modIndex(const QString &id) const;
    //! The mod with the given ID, or nullptr if it isn't in the cache.
    const CachedMod *mod(const QString &id) const;

    //! Adds a CachedMod for the given Steam Workshop details.
    //! Returns the mod if successful, or nullptr otherwise.
    //! If the mod is already in the cache, returns nullptr as the result wouldn't be a non-downloaded mod.
    const CachedMod *addUnloaded(const SteamModInfo &steamInfo);
    //! Extracts the zip contents into a new or existing CachedVersion
    const CachedVersion *addZipVersion(const SteamModInfo &steamInfo, QIODevice &zipFile, QString *errorInfo = nullptr);
    //! Copies the given folder's contents into a new or existing CachedVersion
    const CachedVersion *addModVersion(const QString &modId, const QString &versionId, const QString &folderPath, QString *errorInfo = nullptr);
    //! Refreshes all mods from disk to the specified level.
    void refresh(RefreshLevel level = FULL);
    //! Refreshes and returns the specific mod version from disk. Nullptr if not present.
    const CachedVersion *refreshVersion(const QString &modId, const QString &versionId, RefreshLevel level = FULL);
    //! Re-sorts the cache and persists metadata to disk.
    void saveMetadata();

    //! Finds the currently installed version by hash and set its installed flag.
    //! Returns the version, or nullptr if there is no match in the cache.
    //! If an expected version ID is provided, that version is compared first.
    const CachedVersion *markInstalledVersion(const QString &modId, const QString &hash, const QString &expectedVersionId = QString());
    //! Clears the given mod's installed version.
    void unmarkInstalledMod(const QString &modId);
    //! Updates the default alias of the mod given by mod ID.
    void setDefaultAlias(const QString &modId, const QString &newAlias);

    ~ModCache();

signals:
    //! Emitted just before new mods are appended to the end of the cache.
    void aboutToAppendMods(const QStringList &newModIds);
    //! Emitted after an append operation has completed.
    void appendedMods();
    //! Emitted before mods are arbitrarily changed. Indicates which mods are affected, or an empty list for all.
    //! Indicates mods, both by their mod ID and current index within the mods list.
    //! Notably adding new versions is a refresh event.
    void aboutToRefresh(const QStringList &pendingModIds = QStringList(), const QList<int> &pendingModIdxs = QList<int>(), ChangeHint hint = NO_HINT);
    //! Emitted after mods are arbitrarily changed.
    void refreshed(const QStringList &updatedModIds = QStringList(), const QList<int> &updatedModIdxs = QList<int>(), ChangeHint hint = NO_HINT);
    //! Emitted after a metadata-only change. Indicates which mods are affected, or an empty list for all.
    void metadataChanged(const QStringList &updatedModIds = QStringList(), const QList<int> &updatedModIdxs = QList<int>());

private:
    std::experimental::propagate_const<std::unique_ptr<Impl>> impl;
};

//! A single mod and all its versions currently in the local mod cache.
//! Copying by value will point to the same shared data. Refreshing the parent ModCache separates the shared data.
class IIMODMANLIBSHARED_EXPORT CachedMod
{
public:
    //! Private implementation. Only accessible to classes in this file.
    class Impl;

    CachedMod(const ModCache::Impl &cache, const QString id = QString());

    const QString &id() const;
    const ModInfo &info() const;
    const QString &defaultAlias() const;
    const QList<CachedVersion> &versions() const;
    bool downloaded() const;

    bool containsVersion(const QString &versionId) const;
    bool containsVersion(const QDateTime &versionTime) const;
    //! The position of the version with the given ID in this mod's versions list,
    //! or -1 if it isn't in the cache.
    int versionIndex(const QString &versionId) const;
    int versionIndex(const QDateTime &versionId) const;
    //! The version with the given ID, or nullptr if it isn't in the cache.
    const CachedVersion *version(const QString &versionId) const;
    //! The latest version, or nullptr if no versions are available.
    const CachedVersion *latestVersion() const;
    //! The currently installed version, or nullptr if no versions is installed.
    //! Will also be nullptr if a ModList hasn't refreshed on this ModCache.
    const CachedVersion *installedVersion() const;
    //! The version with the given hash, if present.
    const CachedVersion *versionFromHash(const QString &hash, const QString &expectedVersionId = QString()) const;

private:
    friend ModCache;
    std::shared_ptr<Impl> impl_;

    // Wrap the pointer manually, to preserve CopyConstructable/CopyAssignable.
    inline const Impl *impl() const { return impl_.get(); };
    inline Impl *impl() { return impl_.get(); };
};

//! A single version of a mod in the local mod cache.
//! Copying by value will point to the same shared data. Refreshing the parent ModCache or CachedMod separates the shared data.
class IIMODMANLIBSHARED_EXPORT CachedVersion
{
public:
    //! Private implementation. Only accessible to classes in this file.
    class Impl;

    CachedVersion(const ModCache::Impl &cache, const QString &modId, const QString &versionId);

    const QString &id() const;
    const QString &modId() const;
    const ModInfo &info() const;
    const std::optional<QDateTime> timestamp() const;
    const std::optional<QString> version() const;
    bool installed() const;
    const QString &hash() const;

    const QString toString(StringFormat format = FORMAT_SHORT) const;
    const SpecMod asSpec() const;

    QString path() const;

private:
    friend ModCache;
    friend CachedMod;
    std::shared_ptr<Impl> impl_;

    // Wrap the pointer manually, to preserve CopyConstructable/CopyAssignable.
    inline const Impl *impl() const { return impl_.get(); };
    inline Impl *impl() { return impl_.get(); };
};

} // namespace iimodmanager

#endif // IIMODMANAGER_MODCACHE_H
