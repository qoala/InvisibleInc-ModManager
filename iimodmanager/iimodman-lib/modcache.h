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

class IIMODMANLIBSHARED_EXPORT CachedMod
{
public:
    CachedMod(const ModCache::Impl &cache, const QString &id);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    inline bool downloaded() const { return !versions_.empty(); };

    inline const QList<CachedVersion> &versions() const { return versions_; };
    bool containsVersion(const QString &versionId) const;
    bool containsVersion(const QDateTime &versionTime) const;
    //! The version with the given ID, or nullptr if it isn't in the cache.
    const CachedVersion *version(const QString &versionId) const;
    //! The latest version, or nullptr if no versions are available.
    const CachedVersion *latest() const;

    bool refresh(ModCache::RefreshLevel = ModCache::FULL);
    bool updateFromSteam(const SteamModInfo &steamInfo);

    CachedMod &operator = (const CachedMod &);

private:
    const ModCache::Impl &cache;
    QString id_;
    QList<CachedVersion> versions_;
    ModInfo info_;

    bool readModManFile(QIODevice &file);
    bool writeModManFile(QIODevice &file);
};

class IIMODMANLIBSHARED_EXPORT CachedVersion
{
public:
    CachedVersion(const ModCache::Impl &cache, const QString &modId, const QString &versionId);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    inline const std::optional<QDateTime> timestamp() const { return timestamp_; };
    inline const std::optional<QString> version() const { return version_; };
    const QString &hash() const;

    const QString toString() const;

    bool refresh(ModCache::RefreshLevel = ModCache::FULL);

    CachedVersion &operator = (const CachedVersion &);

private:
    const ModCache::Impl &cache;
    const QString modId;
    QString id_;
    ModInfo info_;
    std::optional<QDateTime> timestamp_;
    std::optional<QString> version_;
    mutable QString hash_;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_MODCACHE_H
