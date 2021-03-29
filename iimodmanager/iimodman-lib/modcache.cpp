#include "modcache.h"
#include "modsignature.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

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

// file-visibility:
    inline QString modPath(const QString &modId) const;
    inline QString modVersionPath(const QString &modId, const QString &versionId) const;

private:
    const ModManConfig &config_;
    //! All cached mods.
    QList<CachedMod> mods_;
    //! Index of mods by mod ID.
    QMap<QString, qsizetype> modIds_;

    void refreshIndex();
};

//! Private implementation of CachedMod.
//! Additionally exposes mutators called by its parent ModCache;
class CachedMod::Impl
{
public:
    Impl(const ModCache::Impl &cache, const QString &id);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    inline const QList<CachedVersion> &versions() const { return versions_; };

// file-visibility:
    bool refresh(ModCache::RefreshLevel = ModCache::FULL);
    bool updateFromSteam(const SteamModInfo &steamInfo);
    const CachedVersion *markInstalledVersion(const QString &hash, const QString expectedVersionId);

private:
    const ModCache::Impl &cache;
    QString id_;
    QList<CachedVersion> versions_;
    ModInfo info_;

    CachedVersion *findVersionByHash(const QString &hash, const QString expectedVersionId);

    bool readModManFile(QIODevice &file);
    bool writeModManFile(QIODevice &file);
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
    const QString &hash() const;

// file-visibility:
    bool refresh(ModCache::RefreshLevel = ModCache::FULL);

private:
    const ModCache::Impl &cache;
    const QString modId;
    QString id_;
    ModInfo info_;
    std::optional<QDateTime> timestamp_;
    std::optional<QString> version_;

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

const CachedMod *ModCache::Impl::addUnloaded(const SteamModInfo &steamInfo)
{
    if (steamInfo.id.isEmpty())
        return nullptr;

    QString modId = steamInfo.modId();
    if (modIds_.contains(modId))
        return nullptr;

    const qsizetype position = mods_.size();
    CachedMod &mod = mods_.emplaceBack(*this, modId);
    if (mod.impl()->updateFromSteam(steamInfo))
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

void ModCache::Impl::refresh(RefreshLevel level)
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
        if (!mod.impl()->refresh(level))
            mods_.removeLast();
    }

    refreshIndex();

    qCDebug(modcache).noquote().nospace() << "cache:refresh() End mods:" << mods_.size();
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

CachedMod::CachedMod(const ModCache::Impl &cache, const QString &id)
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
    for (auto version : impl()->versions())
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
    for (const CachedVersion &version : impl()->versions())
    {
        if (version.id() == versionId)
            return &version;
    }
    return nullptr;
}

const CachedVersion *CachedMod::latest() const
{
    auto versions = impl()->versions();
    if (versions.isEmpty())
        return nullptr;
    return &versions.first();
}

CachedMod::Impl::Impl(const ModCache::Impl &cache, const QString &id)
    : cache(cache), id_(id)
{}

bool CachedMod::Impl::refresh(ModCache::RefreshLevel level)
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
        if (version.impl()->refresh(versionLevel))
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

bool CachedMod::Impl::updateFromSteam(const SteamModInfo &steamInfo)
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

const CachedVersion *CachedMod::Impl::markInstalledVersion(const QString &hash, const QString expectedVersionId)
{
    CachedVersion *installedVersion = findVersionByHash(hash, expectedVersionId);

    return installedVersion;
}

CachedVersion *CachedMod::Impl::findVersionByHash(const QString &hash, const QString expectedVersionId)
{
    // Check the expected version first, to avoid hashing folders unnecessarily.
    if (!expectedVersionId.isEmpty())
    {
        for (CachedVersion &version : versions_)
        {
            if (version.id() == expectedVersionId)
            {
                if (version.hash() == hash)
                    return &version;
                break;
            }
        }
    }

    for (CachedVersion &version : versions_)
    {
        if (version.hash() == hash)
            return &version;
    }
    return nullptr;
}

bool CachedMod::Impl::readModManFile(QIODevice &file)
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

bool CachedMod::Impl::writeModManFile(QIODevice &file)
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
    : cache(cache), modId(modId), id_(versionId)
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
