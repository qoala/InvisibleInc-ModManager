#include "modcache.h"
#include "modsignature.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

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

const QJsonDocument readJSON(QIODevice &file)
{
    QByteArray rawData = file.readAll();
    file.close();
    QJsonParseError errors;
    const QJsonDocument json = QJsonDocument::fromJson(rawData, &errors);

    if (json.isNull())
    {
        qCWarning(modcache) << "JSON Parse Error:", errors.errorString();
        qCDebug(modcache) << rawData;
    }
    return json;
}

ModCache::ModCache(const ModManConfig &config, QObject *parent)
    : QObject(parent), config_(config)
{}

bool ModCache::contains(const QString &id) const
{
    return modIds_.contains(id);
}

const CachedMod *ModCache::mod(const QString &id) const
{
    if (modIds_.contains(id))
        return &mods_.at(modIds_[id]);
    return nullptr;
}

QString ModCache::modPath(const ModManConfig &config, const QString &modId)
{
    QDir cacheDir(config.cachePath());
    return cacheDir.absoluteFilePath(modId);
}

QString ModCache::modVersionPath(const ModManConfig &config, const QString &modId, const QString &versionId)
{
    QDir cacheDir(config.cachePath());
    QDir modDir(cacheDir.absoluteFilePath(modId));
    return modDir.absoluteFilePath(versionId);
}

QString ModCache::modVersionPath(const ModManConfig &config, const QString &modId, const QDateTime &versionTime)
{
    return modVersionPath(config, modId, formatVersionTime(versionTime));
}

const CachedMod *ModCache::addUnloaded(const SteamModInfo &steamInfo)
{
    if (steamInfo.id.isEmpty())
        return nullptr;

    QString modId = steamInfo.modId();
    if (modIds_.contains(modId))
        return nullptr;

    const qsizetype position = mods_.size();
    CachedMod &mod = mods_.emplaceBack(*this, modId);
    if (mod.updateFromSteam(steamInfo))
    {
        modIds_[mod.id()] = position;
        return &mod;
    }
    else
    {
        mods_.removeLast();
        return nullptr;
    }
}

void ModCache::refresh(RefreshLevel level)
{
    QDir cacheDir(config_.cachePath());
    qCDebug(modcache).noquote() << "cache:refresh() Start" << cacheDir.path();

    cacheDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    cacheDir.setSorting(QDir::Name);
    QStringList modIds = cacheDir.entryList();
    mods_.clear();
    mods_.reserve(modIds.size());
    for (auto modId : modIds)
    {
        CachedMod &mod = mods_.emplaceBack(*this, modId);
        if (!mod.refresh(level))
            mods_.removeLast();
    }

    refreshIndex();

    qCDebug(modcache).noquote().nospace() << "cache:refresh() End mods:" << mods_.size();
}

void ModCache::refreshIndex()
{
    modIds_.clear();
    for (qsizetype i = 0; i < mods_.size(); ++i)
    {
        const CachedMod &mod = mods_.at(i);
        const QString &modId = mod.id();
        modIds_[modId] = i;
    }
}

CachedMod::CachedMod(const ModCache &cache, const QString &id)
    : cache(cache), id_(id)
{}

bool CachedMod::containsVersion(const QString &versionId) const
{
    for (auto version : versions_)
    {
        if (version.id() == versionId)
            return true;
    }
    return false;
}

bool CachedMod::containsVersion(const QDateTime &versionTime) const
{
    return containsVersion(formatVersionTime(versionTime));
}

const CachedVersion *CachedMod::version(const QString &versionId) const
{
    for (const CachedVersion &version : versions_)
    {
        if (version.id() == versionId)
            return &version;
    }
    return nullptr;
}

const CachedVersion *CachedMod::latest() const
{
    if (versions_.isEmpty())
        return nullptr;
    return &versions_.first();
}

bool CachedMod::refresh(ModCache::RefreshLevel level)
{
    QDir modDir(cache.modPath(id_));

    modDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    modDir.setSorting(QDir::Name | QDir::Reversed);
    QFileInfoList versionPaths = modDir.entryInfoList();
    versions_.clear();
    versions_.reserve(versionPaths.size());
    ModCache::RefreshLevel versionLevel = level == ModCache::LATEST_ONLY ? ModCache::FULL : level;
    for (auto versionPath : versionPaths)
    {
        CachedVersion &version = versions_.emplaceBack(cache, id(), versionPath.fileName());
        if (version.refresh(versionLevel))
        {
            if (level == ModCache::LATEST_ONLY)
                versionLevel = ModCache::ID_ONLY;
        }
        else
        {
            versions_.removeLast();
        }
    }

    qCDebug(modcache).noquote().nospace() << QString("mod:refresh(%1)").arg(id_) << " versions:" << versions_.size();

    if (versions_.size() > 0)
    {
        info_ = versions_.first().info();
        return true;
    }
    else if (modDir.exists("modman.json"))
    {
        // A modman.json file is sufficient to describe a mod, even without any downloaded versions.
        QFile modManFile(modDir.filePath("modman.json"));
        return readModManFile(modManFile);
    }
    return false;
}

bool CachedMod::updateFromSteam(const SteamModInfo &steamInfo)
{
    bool changed = false;
    if (versions_.empty())
    {
        changed = true;
        info_ = ModInfo(id_, steamInfo.title);
    }

    if (changed)
    {
        QDir modDir(cache.modPath(id_));
        if (modDir.mkpath("."))
        {
            QFile modManFile(modDir.filePath("modman.json"));
            qCDebug(modcache).noquote() << QString("updateMod(%1)").arg(id_) << "Writing to" << modDir.filePath("modman.json");
            if (!writeModManFile(modManFile))
            {
                qCWarning(modcache).noquote() << QString("updateMod(%1)").arg(id_) << "Couldn't write" << modDir.filePath("modman.json");
            }
        }
        else
        {
            qCWarning(modcache).noquote() << QString("updateMod(%1)").arg(id_) << "Couldn't create directory" << modDir.path();
        }
    }
    return changed;
}

CachedMod &CachedMod::operator =(const CachedMod &o)
{
   assert(&cache == &o.cache);
   id_ = o.id_;
   versions_ = o.versions_;
   info_ = o.info_;
   return *this;
}

bool CachedMod::readModManFile(QIODevice &file)
{
    if (file.isOpen() || file.open(QIODevice::ReadOnly))
    {
        const QJsonDocument json = readJSON(file);
        if (json.isNull())
            return false;

        const QJsonObject root = json.object();

        QString name;
        const QJsonValue nameJson = root.value("modName");
        if (!nameJson.isUndefined())
            name = root.value("modName").toString();

        info_ = ModInfo(id_, name);
        return true;
    }
    return false;
}

bool CachedMod::writeModManFile(QIODevice &file)
{
    QJsonObject root;
    root["modId"] = id_;
    root["modName"] = info().name();
    QJsonDocument json(root);

    if ((file.isOpen() && file.isWritable()) || file.open(QIODevice::WriteOnly))
    {
        file.write(json.toJson());
        return true;
    }
    return false;
}

CachedVersion::CachedVersion(const ModCache &cache, const QString &modId, const QString &versionId)
    : cache(cache), modId(modId), id_(versionId)
{}

const QString &CachedVersion::hash() const
{
    if (hash_.isEmpty())
        hash_ = ModSignature::hashModPath(cache.modVersionPath(modId, id_));
    return hash_;
}

const QString CachedVersion::toString() const
{
    if (version().has_value())
        return "v" + *version();
    else if (timestamp().has_value())
        return timestamp()->toString();
    else
        return id();

}

bool CachedVersion::refresh(ModCache::RefreshLevel level)
{
    QDir modVersionDir(cache.modVersionPath(modId, id_));
    if (!modVersionDir.exists("modinfo.txt"))
    {
        qCDebug(modcache).noquote() << QString("modversion:refresh(%1,%2)").arg(modId, id_) << "skipped: No modinfo.txt";
        return false;
    }

    hash_.clear();

    if (level == ModCache::ID_ONLY)
        return true;

    QFile infoFile = QFile(modVersionDir.filePath("modinfo.txt"));
    info_ = ModInfo::readModInfo(infoFile, modId);
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

CachedVersion &CachedVersion::operator =(const CachedVersion &o)
{
   assert(&cache == &o.cache);
   id_ = o.id_;
   info_ = o.info_;
   timestamp_ = o.timestamp_;
   version_ = o.version_;
   return *this;
}

} // namespace iimodmanager
