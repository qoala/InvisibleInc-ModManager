#include "modcache.h"

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

CachedMod::CachedMod(const ModCache &cache)
    : cache(cache)
{}

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

bool CachedMod::updateFromSteam(const SteamModInfo &steamInfo)
{
    bool changed = false;
    if (versions_.empty())
    {
        changed = true;
        id_ = steamInfo.modId();
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

ModCache::ModCache(const ModManConfig &config, QObject *parent)
    : QObject(parent), config_(config)
{}

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

const CachedMod &ModCache::addUnloaded(const SteamModInfo &steamInfo)
{
    const qsizetype position = mods_.size();
    CachedMod &mod = mods_.emplaceBack(*this);
    mod.updateFromSteam(steamInfo);

    modIds_[mod.id()] = position;
    return mod;
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
            mod = &mods_.emplaceBack(*this);
        }
        lastValid = mod->refresh(modPath.filePath());
    }
    if (!lastValid)
    {
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

} // namespace iimodmanager
