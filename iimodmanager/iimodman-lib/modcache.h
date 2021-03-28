#ifndef IIMODMANAGER_MODCACHE_H
#define IIMODMANAGER_MODCACHE_H

#include "moddownloader.h"
#include "modinfo.h"

#include <QDateTime>
#include <QList>
#include <QLoggingCategory>
#include <QMap>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(modcache)

class CachedMod;
class CachedVersion;

//! Mods cached locally, but not installed. May be downloaded or just have an ID-to-name registration.
class IIMODMANLIBSHARED_EXPORT ModCache : public QObject
{
public:
    enum RefreshLevel
    {
        FULL,
        ID_ONLY,
        LATEST_ONLY,
    };

    ModCache(const ModManConfig &config, QObject *parent = nullptr);

    inline const QList<CachedMod> mods() const { return mods_; };
    bool contains(const QString &id) const;
    const CachedMod &mod(const QString &id) const;

    inline QString modPath(const QString &modId) const { return modPath(config_, modId); };
    inline QString modVersionPath(const QString &modId, const QString &versionId) const { return modVersionPath(config_, modId, versionId); };
    inline QString modVersionPath(const QString &modId, const QDateTime &versionTime) const { return modVersionPath(config_, modId, versionTime); };
    static QString modPath(const ModManConfig &config, const QString &modId);
    static QString modVersionPath(const ModManConfig &config, const QString &modId, const QString &versionId);
    static QString modVersionPath(const ModManConfig &config, const QString &modId, const QDateTime &versionTime);

    const CachedMod &addUnloaded(const SteamModInfo &steamInfo);
    void refresh(RefreshLevel = FULL);

private:
    const ModManConfig &config_;
    //! All cached mods.
    QList<CachedMod> mods_;
    //! Index of mods by mod ID.
    QMap<QString, qsizetype> modIds_;

    void refreshIndex();
};

class IIMODMANLIBSHARED_EXPORT CachedMod
{
public:
    CachedMod(const ModCache &cache, const QString &id);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    inline bool downloaded() const { return !versions_.empty(); };

    inline const QList<CachedVersion> &versions() const { return versions_; };
    const CachedVersion *version(const QString &versionId) const;
    const CachedVersion &latest() const { return versions_.first(); };
    bool containsVersion(const QString &versionId) const;
    bool containsVersion(const QDateTime &versionTime) const;

    bool refresh(ModCache::RefreshLevel = ModCache::FULL);
    bool updateFromSteam(const SteamModInfo &steamInfo);

    CachedMod &operator = (const CachedMod &);

private:
    const ModCache &cache;
    QString id_;
    QList<CachedVersion> versions_;
    ModInfo info_;

    bool readModManFile(QIODevice &file);
    bool writeModManFile(QIODevice &file);
};

class IIMODMANLIBSHARED_EXPORT CachedVersion
{
public:
    CachedVersion(const ModCache &cache, const QString &modId, const QString &versionId);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    inline const std::optional<QDateTime> timestamp() const { return timestamp_; };
    inline const std::optional<QString> version() const { return version_; };
    const QString &hash() const;

    const QString toString() const;

    bool refresh(ModCache::RefreshLevel = ModCache::FULL);

    CachedVersion &operator = (const CachedVersion &);

private:
    const ModCache &cache;
    const QString modId;
    QString id_;
    ModInfo info_;
    std::optional<QDateTime> timestamp_;
    std::optional<QString> version_;
    mutable QString hash_;
};

inline bool ModCache::contains(const QString &id) const
{
    return modIds_.contains(id);
}

inline const CachedMod &ModCache::mod(const QString &id) const
{
    return mods_.at(modIds_.value(id));
}

} // namespace iimodmanager

#endif // IIMODMANAGER_MODCACHE_H
