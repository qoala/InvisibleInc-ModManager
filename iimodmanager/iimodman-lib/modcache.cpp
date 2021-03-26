#include "modcache.h"

#include <QDir>

namespace iimodmanager {

Q_LOGGING_CATEGORY(modcache, "modcache", QtWarningMsg)

// Folder Structure: {cachePath}/workshop-{steamId}/{versionTime}/
// Time format is ISO8601, but with ':' replaced with '_' to be a valid folder name on Windows.
const QString formatVersionTime(const QDateTime &versionTime)
{
    return versionTime.toString(Qt::ISODate).replace(':', '_');
}

const QDateTime parseVersionTime(const QString &versionId)
{
    return QDateTime::fromString(QString(versionId).replace('_', ':'), Qt::ISODate);
}

bool CachedMod::containsVersion(const QString &id) const
{
    for (auto version : versions_)
    {
        if (version.id() == id)
        {
            return true;
        }
    }
    return false;
}

bool CachedMod::containsVersion(const QDateTime &versionTime) const
{
    return containsVersion(formatVersionTime(versionTime));
}

ModCache::ModCache(const ModManConfig &config, QObject *parent)
    : QObject(parent), config_(config)
{}

QString ModCache::modPath(const ModManConfig &config, const QString &modId, const QString &versionId)
{
    QDir cacheDir(config.cachePath());
    QDir modDir(cacheDir.absoluteFilePath(modId));
    return modDir.absoluteFilePath(versionId);
}

QString ModCache::modPath(const ModManConfig &config, const QString &modId, const QDateTime &versionTime)
{
    return modPath(config, modId, formatVersionTime(versionTime));
}

void ModCache::refresh()
{
    QDir cacheDir(config_.cachePath());
    qCDebug(modcache).noquote() << "cache:refresh() Start" << cacheDir.path();

    cacheDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    cacheDir.setSorting(QDir::Name);
    QFileInfoList modPaths = cacheDir.entryInfoList();
    mods_.clear();
    mods_.reserve(modPaths.size());
    CachedMod *mod;
    bool lastValid = true;
    for (auto modPath : modPaths)
    {
        if (lastValid)
        {
            mod = &mods_.emplaceBack();
        }
        lastValid = mod->refresh(modPath.filePath());
    }
    if (!lastValid)
    {
        mods_.removeLast();
    }

    // Build an index by mod ID.
    modIds_.clear();
    for (qsizetype i = 0; i < mods_.size(); ++i)
    {
        const CachedMod &mod = mods_.at(0);
        modIds_[mod.id()] = i;
    }

    qCDebug(modcache).noquote().nospace() << "cache:refresh() End mods:" << mods_.size();
}

bool CachedMod::refresh(const QString &modPath)
{
    QDir modDir(modPath);
    id_ = modDir.dirName();

    modDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    modDir.setSorting(QDir::Name | QDir::Reversed);
    QFileInfoList versionPaths = modDir.entryInfoList();
    versions_.clear();
    versions_.reserve(versionPaths.size());
    CachedVersion *version;
    bool lastValid = true;
    for (auto versionPath : versionPaths)
    {
        if (lastValid)
        {
            version = &versions_.emplaceBack();
        }
        lastValid = version->refresh(versionPath.filePath(), id_);
    }
    if (!lastValid)
    {
        versions_.removeLast();
    }

    qCDebug(modcache).noquote().nospace() << QString("mod:refresh(%1)").arg(id_) << " versions:" << versions_.size();

    return versions_.size() > 0;
}

bool CachedVersion::refresh(const QString &modVersionPath, const QString &modId)
{
    QDir modVersionDir(modVersionPath);
    id_ = modVersionDir.dirName();

    if (!modVersionDir.exists("modinfo.txt"))
    {
        qCDebug(modcache).noquote() << QString("modversion:refresh(%1,%2)").arg(modId, id_) << "skipped: No modinfo.txt";
        return false;
    }

    QFile infoFile = QFile(modVersionDir.filePath("modinfo.txt"));
    info_ = Mod::readModInfo(infoFile, modId);
    infoFile.close();
    if (!info().version().isEmpty())
    {
        version_ = info().version();
    }
    else
    {
        version_.reset();
    }

    timestamp_ = parseVersionTime(id_);
    if (!timestamp_->isValid())
    {
        timestamp_.reset();
    }

    qCDebug(modcache).noquote().nospace() << QString("modversion:refresh(%1,%2)").arg(modId, id_) << " version=" << (version_.has_value() ? *version_ : "");
    return true;
}

} // namespace iimodmanager
