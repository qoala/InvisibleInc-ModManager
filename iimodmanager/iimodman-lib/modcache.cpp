#include "modcache.h"
#include "moddownloader.h"
#include "modinfo.h"
#include "modsignature.h"

#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QMap>

namespace iimodmanager {

Q_LOGGING_CATEGORY(modcache, "modcache", QtWarningMsg)

//! Private implementation of ModCache.
//! Additionally exposes methods to the CachedMod/CachedVersion children defined in this file.
class ModCache::Impl
{
public:
    Impl(const ModManConfig &config);

    inline const QList<CachedMod> mods() const { return mods_; };
    bool contains(const QString &id) const;
    const CachedMod *mod(const QString &id) const;

    CachedMod *mod(const QString &id);

    const CachedMod *addUnloaded(const SteamModInfo &steamInfo);
    void refresh(RefreshLevel = FULL);
    inline void save();

// file-visibility:
    inline QString modPath(const QString &modId) const;
    inline QString modVersionPath(const QString &modId, const QString &versionId) const;

private:
    const ModManConfig &config_;
    //! All cached mods.
    QList<CachedMod> mods_;
    //! Index of mods by mod ID.
    QMap<QString, qsizetype> modIds_;

    void sortMods();
    void refreshIndex();
    bool readModManDb();
    bool writeModManDb();
};

//! Private implementation of CachedMod.
//! Additionally exposes mutators called by its parent ModCache;
class CachedMod::Impl
{
public:
    Impl(const ModCache::Impl &cache, const QString id = QString());

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    inline const QList<CachedVersion> &versions() const { return versions_; };
    inline const CachedVersion *installedVersion() const { return installedVersion_; };

// file-visibility:
    CachedVersion *version(const QString &versionId);

    bool refresh(ModCache::RefreshLevel = ModCache::FULL);
    bool updateFromSteam(const SteamModInfo &steamInfo);
    const CachedVersion *markInstalledVersion(const QString &hash, const QString expectedVersionId);

    bool readDb(const QJsonObject &modObject);
    void writeDb(QJsonObject &modObject) const;

private:
    const ModCache::Impl &cache;
    QString id_;
    QList<CachedVersion> versions_;
    ModInfo info_;

    CachedVersion *installedVersion_;

    CachedVersion *findVersionByHash(const QString &hash, const QString expectedVersionId);
};

//! Private implementation of CachedVersion.
//! Additionally exposes mutators called by its parent ModCache and CachedMod;
class CachedVersion::Impl
{
public:
    Impl(const ModCache::Impl &cache, const QString &modId, const QString &versionId);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    inline const std::optional<QDateTime> timestamp() const { return timestamp_; };
    inline const std::optional<QString> version() const { return version_; };
    inline bool installed() const { return installed_; };
    const QString &hash() const;

// file-visibility:
    void setInstalled(bool value) { installed_ = value; };
    bool refresh(ModCache::RefreshLevel = ModCache::FULL);

private:
    const ModCache::Impl &cache;
    const QString modId;
    QString id_;
    ModInfo info_;
    std::optional<QDateTime> timestamp_;
    std::optional<QString> version_;

    bool installed_;
    mutable QString hash_;
};

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

const QJsonObject readJSON(QIODevice &file)
{
    QByteArray rawData = file.readAll();
    file.close();
    QJsonParseError errors;
    const QJsonDocument json = QJsonDocument::fromJson(rawData, &errors);

    if (json.isNull())
    {
        qCWarning(modcache) << "JSON Parse Error:", errors.errorString();
        qCDebug(modcache) << rawData;
        return QJsonObject();
    }
    if (!json.isObject())
    {
        qCWarning(modcache) << "JSON: not an object";
        return QJsonObject();
    }

    return json.object();
}

bool compareModIds(const CachedMod &a, const CachedMod &b)
{
    return a.id() < b.id();
}

ModCache::ModCache(const ModManConfig &config, QObject *parent)
    : QObject(parent), impl{std::make_unique<Impl>(config)}
{}

const QList<CachedMod> ModCache::mods() const
{
    return impl->mods();
}

bool ModCache::contains(const QString &id) const
{
    return impl->contains(id);
}

const CachedMod *ModCache::mod(const QString &id) const
{
    return impl->mod(id);
}

QString ModCache::modVersionPath(const ModManConfig &config, const QString &modId, const QDateTime &versionTime)
{
    QDir cacheDir(config.cachePath());
    QDir modDir(cacheDir.absoluteFilePath(modId));
    return modDir.absoluteFilePath(formatVersionTime(versionTime));
}

const CachedMod *ModCache::addUnloaded(const SteamModInfo &steamInfo)
{
    return impl->addUnloaded(steamInfo);
}

void ModCache::refresh(ModCache::RefreshLevel level)
{
    impl->refresh(level);
}

void ModCache::saveMetadata()
{
    impl->save();
}

const CachedVersion *ModCache::markInstalledVersion(const QString &modId, const QString &hash, const QString expectedVersionId)
{
    CachedMod *mod = impl->mod(modId);
    if (mod)
        return mod->impl()->markInstalledVersion(hash, expectedVersionId);
    return nullptr;
}

ModCache::~ModCache() = default;

ModCache::Impl::Impl(const ModManConfig &config)
    : config_(config)
{}

bool ModCache::Impl::contains(const QString &id) const
{
    return modIds_.contains(id);
}

const CachedMod *ModCache::Impl::mod(const QString &id) const
{
    if (modIds_.contains(id))
        return &mods_.at(modIds_[id]);
    return nullptr;
}

CachedMod *ModCache::Impl::mod(const QString &id)
{
    if (modIds_.contains(id))
        return &mods_[modIds_[id]];
    return nullptr;
}

const CachedMod *ModCache::Impl::addUnloaded(const SteamModInfo &steamInfo)
{
    if (steamInfo.id.isEmpty())
        return nullptr;

    QString modId = steamInfo.modId();
    if (modIds_.contains(modId))
        return nullptr;

    CachedMod mod(*this, modId);
    if (mod.impl()->updateFromSteam(steamInfo))
    {
        modIds_[mod.id()] = mods_.size();
        mods_.append(mod);
        return &mods_.last();
    }

    return nullptr;
}

void ModCache::Impl::refresh(RefreshLevel level)
{
    QDir cacheDir(config_.cachePath());
    qCDebug(modcache).noquote() << "cache:refresh() Start" << cacheDir.path();

    mods_.clear();
    readModManDb();
    sortMods();

    cacheDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    cacheDir.setSorting(QDir::Name);
    const QStringList modIds = cacheDir.entryList();
    mods_.reserve(mods_.size() + modIds.size());
    for (auto modId : modIds)
    {
        if (CachedMod *m = mod(modId))
        {
            m->impl()->refresh(level);
        }
        else
        {
            CachedMod newMod(*this, modId);
            if (newMod.impl()->refresh(level))
                mods_.append(newMod);
        }
    }

    sortMods();

    qCDebug(modcache).noquote().nospace() << "cache:refresh() End mods:" << mods_.size();
}

void ModCache::Impl::save()
{
    sortMods();
    writeModManDb();
}

QString ModCache::Impl::modPath(const QString &modId) const
{
    QDir cacheDir(config_.cachePath());
    return cacheDir.absoluteFilePath(modId);
}

QString ModCache::Impl::modVersionPath(const QString &modId, const QString &versionId) const
{
    QDir cacheDir(config_.cachePath());
    QDir modDir(cacheDir.absoluteFilePath(modId));
    return modDir.absoluteFilePath(versionId);
}

void ModCache::Impl::sortMods()
{
    std::sort(mods_.begin(), mods_.end(), compareModIds);
    refreshIndex();
}

void ModCache::Impl::refreshIndex()
{
    modIds_.clear();
    for (qsizetype i = 0; i < mods_.size(); ++i)
    {
        const CachedMod &mod = mods_.at(i);
        const QString &modId = mod.id();
        modIds_[modId] = i;
    }
}

bool ModCache::Impl::readModManDb()
{
    QDir cacheDir(config_.cachePath());
    QFile file(cacheDir.filePath("modmandb.json"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        const QJsonObject root = readJSON(file);
        if (root.isEmpty())
            return false;

        if (root.contains("mods") && root["mods"].isArray())
        {
            const QJsonArray modsArray = root["mods"].toArray();
            mods_.reserve(modsArray.size());
            for (const QJsonValue &v : modsArray)
            {
                const QJsonObject modObject = v.toObject();
                CachedMod mod(*this);
                if (mod.impl()->readDb(modObject))
                    mods_.append(mod);
            }
        }
        return true;
    }
    return false;
}

bool ModCache::Impl::writeModManDb()
{
    QJsonObject root;

    QJsonArray modsArray;
    for (const CachedMod &mod : mods_)
    {
        QJsonObject modObject;
        mod.impl()->writeDb(modObject);
        modsArray.append(modObject);
    }
    root["mods"] = modsArray;

    QJsonDocument json(root);

    QDir cacheDir(config_.cachePath());
    QFile file(cacheDir.filePath("modmandb.json"));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(json.toJson());
        return true;
    }
    return false;
}

CachedMod::CachedMod(const ModCache::Impl &cache, const QString id)
    : impl_{std::make_shared<Impl>(cache, id)}
{}

const QString &CachedMod::id() const
{
    return impl()->id();
}

const ModInfo &CachedMod::info() const
{
    return impl()->info();
}

const QList<CachedVersion> &CachedMod::versions() const
{
    return impl()->versions();
}

bool CachedMod::downloaded() const
{
    return !impl()->versions().isEmpty();
}

bool CachedMod::containsVersion(const QString &versionId) const
{
    for (const CachedVersion &cachedVersion : impl()->versions())
    {
        if (cachedVersion.id() == versionId)
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
    for (const CachedVersion &cachedVersion : impl()->versions())
    {
        if (cachedVersion.id() == versionId)
            return &cachedVersion;
    }
    return nullptr;
}

const CachedVersion *CachedMod::latestVersion() const
{
    const QList<CachedVersion> &versions = impl()->versions();
    if (versions.isEmpty())
        return nullptr;
    return &versions.first();
}

const CachedVersion *CachedMod::installedVersion() const
{
    return impl()->installedVersion();
}

CachedMod::Impl::Impl(const ModCache::Impl &cache, const QString id)
    : cache(cache), id_(id), installedVersion_(nullptr)
{}

CachedVersion *CachedMod::Impl::version(const QString &versionId)
{
    for (CachedVersion &cachedVersion : versions_)
    {
        if (cachedVersion.id() == versionId)
            return &cachedVersion;
    }
    return nullptr;
}

bool CachedMod::Impl::refresh(ModCache::RefreshLevel level)
{
    QDir modDir(cache.modPath(id_));

    QString installedVersionId = installedVersion_ ? installedVersion_->id() : QString();

    modDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    modDir.setSorting(QDir::Name | QDir::Reversed);
    const QFileInfoList versionPaths = modDir.entryInfoList();
    versions_.clear();
    versions_.reserve(versionPaths.size());
    ModCache::RefreshLevel versionLevel = level == ModCache::LATEST_ONLY ? ModCache::FULL : level;
    for (auto versionPath : versionPaths)
    {
        CachedVersion cachedVersion(cache, id(), versionPath.fileName());
        if (cachedVersion.impl()->refresh(versionLevel))
        {
            versions_.append(cachedVersion);
            if (level == ModCache::LATEST_ONLY)
                versionLevel = ModCache::ID_ONLY;
        }
    }

    qCDebug(modcache).noquote().nospace() << QString("mod:refresh(%1)").arg(id_) << " versions:" << versions_.size();

    if (!installedVersionId.isEmpty())
    {
        installedVersion_ = version(installedVersionId);
        if (!installedVersion_)
            qCWarning(modcache) << info_.toString() << "no longer contains installed version after refresh:" << installedVersionId;
    }

    if (versions_.size() > 0)
    {
        info_ = versions_.first().info();
        return true;
    }
    return false;
}

bool CachedMod::Impl::updateFromSteam(const SteamModInfo &steamInfo)
{
    if (versions_.empty())
    {
        info_ = ModInfo(id_, steamInfo.title);
        return !info_.isEmpty();
    }
    return false;
}

const CachedVersion *CachedMod::Impl::markInstalledVersion(const QString &hash, const QString expectedVersionId)
{
    CachedVersion *cachedVersion = findVersionByHash(hash, expectedVersionId);

    if (!cachedVersion)
        return nullptr;

    // Clear flags on any previously installed version.
    if (installedVersion_)
        installedVersion_->impl()->setInstalled(false);

    // Ensure the version's modinfo is available, in case this isn't latest.
    if (cachedVersion->info().isEmpty())
        cachedVersion->impl()->refresh();

    cachedVersion->impl()->setInstalled(true);
    installedVersion_ = cachedVersion;
    return cachedVersion;
}

bool CachedMod::Impl::readDb(const QJsonObject &modObject)
{
    QString name;

    if (modObject.contains("modId") && modObject["modId"].isString())
        id_ = modObject["modId"].toString();
    if (modObject.contains("modName") && modObject["modName"].isString())
        name = modObject["modName"].toString();

    if (!id_.isEmpty() && !name.isEmpty())
    {
        info_ = ModInfo(id_, name);
        return true;
    }
    return false;
}

void CachedMod::Impl::writeDb(QJsonObject &modObject) const
{
    modObject["modId"] = id_;
    modObject["modName"] = info().name();
}

CachedVersion *CachedMod::Impl::findVersionByHash(const QString &hash, const QString expectedVersionId)
{
    // Check the expected version first, to avoid hashing folders unnecessarily.
    if (!expectedVersionId.isEmpty())
    {
        CachedVersion *expectedVersion = version(expectedVersionId);
        if (expectedVersion && expectedVersion->hash() == hash)
            return expectedVersion;
    }

    for (CachedVersion &version : versions_)
    {
        if (version.hash() == hash)
            return &version;
    }
    return nullptr;
}

CachedVersion::CachedVersion(const ModCache::Impl &cache, const QString &modId, const QString &versionId)
    : impl_{std::make_shared<Impl>(cache, modId, versionId)}
{}

const QString &CachedVersion::id() const
{
    return impl()->id();
}

const ModInfo &CachedVersion::info() const
{
    return impl()->info();
}

const std::optional<QDateTime> CachedVersion::timestamp() const
{
    return impl()->timestamp();
}

const std::optional<QString> CachedVersion::version() const
{
    return impl()->version();
}

bool CachedVersion::installed() const
{
    return impl()->installed();
}

const QString &CachedVersion::hash() const
{
    return impl()->hash();
}

const QString CachedVersion::toString() const
{
    if (auto version = impl()->version())
        return "v" + *version;
    else if (auto timestamp = impl()->timestamp())
        return timestamp->toString();
    else
        return impl()->id();

}

CachedVersion::Impl::Impl(const ModCache::Impl &cache, const QString &modId, const QString &versionId)
    : cache(cache), modId(modId), id_(versionId), installed_(false)
{}

const QString &CachedVersion::Impl::hash() const
{
    if (hash_.isEmpty())
        hash_ = ModSignature::hashModPath(cache.modVersionPath(modId, id_));
    return hash_;
}

bool CachedVersion::Impl::refresh(ModCache::RefreshLevel level)
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

    qCDebug(modcache).noquote().nospace() << QString("modversion:refresh(%1,%2)").arg(modId, id_) << " version=" << (version_ ? *version_ : "");
    return true;
}

} // namespace iimodmanager
