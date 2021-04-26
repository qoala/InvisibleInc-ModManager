#ifndef IIMODMANAGER_MODCACHE_H
#define IIMODMANAGER_MODCACHE_H

#include "iimodman-lib_global.h"

#include <experimental/propagate_const>

#include <QLoggingCategory>
#include <QObject>
#include <memory>

class QDateTime;
template <typename T> class QList;


namespace iimodmanager {

class ModInfo;
class ModManConfig;
class SpecMod;
class SteamModInfo;


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
public:
    //! Private implementation. Only accessible to classes in this file.
    class Impl;
    enum RefreshLevel
    {
        FULL,
        ID_ONLY,
        LATEST_ONLY,
    };

    ModCache(const ModManConfig &config, QObject *parent = nullptr);

    const QList<CachedMod> mods() const;
    bool contains(const QString &id) const;
    //! The mod with the given ID, or nullptr if it isn't in the cache.
    const CachedMod *mod(const QString &id) const;

    //! Add a CachedMod for the given Steam Workshop details.
    //! Returns the mod if successful, or nullptr otherwise.
    //! If the mod is already in the cache, returns nullptr as the result wouldn't be a non-downloaded mod.
    const CachedMod *addUnloaded(const SteamModInfo &steamInfo);
    //! Extract the zip contents into a new or existing CachedVersion
    const CachedVersion *addZipVersion(const SteamModInfo &steamInfo, QIODevice &zipFile);
    //! Copy the given folder's contents into a new or existing CachedVersion
    const CachedVersion *addModVersion(const QString &modId, const QString &versionId, const QString &folderPath);
    void refresh(RefreshLevel level = FULL);
    void saveMetadata();

    //! Find the currently installed version by hash and set its installed flag.
    //! Returns the version, or nullptr if there is no match in the cache.
    //! If an expected version ID is provided, that version is compared first.
    const CachedVersion *markInstalledVersion(const QString &modId, const QString &hash, const QString expectedVersionId = QString());

    ~ModCache();
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
    const QList<CachedVersion> &versions() const;
    bool downloaded() const;

    bool containsVersion(const QString &versionId) const;
    bool containsVersion(const QDateTime &versionTime) const;
    //! The version with the given ID, or nullptr if it isn't in the cache.
    const CachedVersion *version(const QString &versionId) const;
    //! The latest version, or nullptr if no versions are available.
    const CachedVersion *latestVersion() const;
    //! The currently installed version, or nullptr if no versions is installed.
    //! Will also be nullptr if a ModList hasn't refreshed on this ModCache.
    const CachedVersion *installedVersion() const;

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
    const ModInfo &info() const;
    const std::optional<QDateTime> timestamp() const;
    const std::optional<QString> version() const;
    bool installed() const;
    const QString &hash() const;

    const QString toString(StringFormat format = FORMAT_SHORT) const;
    const SpecMod asSpec() const;

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
