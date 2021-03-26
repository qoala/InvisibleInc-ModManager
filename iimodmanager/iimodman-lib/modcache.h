#ifndef IIMODMANAGER_MODCACHE_H
#define IIMODMANAGER_MODCACHE_H

# include "mod.h"

#include <QDateTime>
#include <QList>
#include <QLoggingCategory>
#include <QMap>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(modcache)

class ModCache;

class IIMODMANLIBSHARED_EXPORT CachedVersion
{
public:
    inline const QString &id() const { return id_; };
    inline const Mod &info() const { return info_; };
    inline const std::optional<QDateTime> timestamp() const { return timestamp_; };
    inline const std::optional<QString> version() const { return version_; };

    bool refresh(const QString &modVersionPath, const QString &modId);

private:
    QString id_;
    Mod info_;
    std::optional<QDateTime> timestamp_;
    std::optional<QString> version_;
};

class IIMODMANLIBSHARED_EXPORT CachedMod
{
public:
    inline const QString &id() const { return id_; };
    inline const Mod &info() const { return versions_.last().info(); };

    inline const QList<CachedVersion> &versions() const { return versions_; };
    bool containsVersion(const QString &id) const;
    bool containsVersion(const QDateTime &versionTime) const;

    bool refresh(const QString &modPath);

private:
    QString id_;
    QList<CachedVersion> versions_;
};

class IIMODMANLIBSHARED_EXPORT ModCache : public QObject
{
public:
    ModCache(const ModManConfig &config, QObject *parent = nullptr);

    inline const QList<CachedMod> mods() const { return mods_; };
    bool contains(const QString &id) const;
    const CachedMod &mod(const QString &id) const;

    inline QString modPath(const QString &modId, const QString &versionId) { return modPath(config_, modId, versionId); };
    inline QString modPath(const QString &modId, const QDateTime &versionTime) { return modPath(config_, modId, versionTime); };
    static QString modPath(const ModManConfig &config, const QString &modId, const QString &versionId);
    static QString modPath(const ModManConfig &config, const QString &modId, const QDateTime &versionTime);

    void refresh();

private:
    const ModManConfig &config_;
    QList<CachedMod> mods_;
    QMap<QString, qsizetype> modIds_;
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
