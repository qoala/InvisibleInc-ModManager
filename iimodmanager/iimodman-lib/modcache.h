#ifndef IIMODMANAGER_MODCACHE_H
#define IIMODMANAGER_MODCACHE_H

#include "moddownloader.h"
#include "modinfo.h"

#include <QDateTime>
#include <QList>
#include <QLoggingCategory>
#include <QMap>
#include <experimental/propagate_const>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(modcache)

class CachedMod;
class CachedVersion;

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

    static QString modVersionPath(const ModManConfig &config, const QString &modId, const QDateTime &versionTime);

    //! Add a CachedMod for the given Steam Workshop details.
    //! Returns the mod if successful, or nullptr otherwise.
    //! If the mod is already in the cache, returns nullptr as the result wouldn't be a non-downloaded mod.
    const CachedMod *addUnloaded(const SteamModInfo &steamInfo);
    void refresh(RefreshLevel level = FULL);

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

    CachedMod(const ModCache::Impl &cache, const QString &id);

    const QString &id() const;
    const ModInfo &info() const;
    const QList<CachedVersion> &versions() const;
    bool downloaded() const;

    bool containsVersion(const QString &versionId) const;
    bool containsVersion(const QDateTime &versionTime) const;
    //! The version with the given ID, or nullptr if it isn't in the cache.
    const CachedVersion *version(const QString &versionId) const;
    //! The latest version, or nullptr if no versions are available.
    const CachedVersion *latest() const;

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
    const QString &hash() const;

    const QString toString() const;

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
