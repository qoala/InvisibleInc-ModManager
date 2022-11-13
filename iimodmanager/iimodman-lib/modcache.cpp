#include "fileutils.h"
#include "modcache.h"
#include "moddownloader.h"
#include "modinfo.h"
#include "modsignature.h"
#include "modspec.h"

#include <JlCompress.h>
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

enum OperationContext
{
    COMPLETE_OP,
    PARTIAL_OP,
};

//! Private implementation of ModCache.
//! Additionally exposes methods to the CachedMod/CachedVersion children defined in this file.
class ModCache::Impl
{
public:
    Impl(const ModManConfig &config);

    inline const QList<CachedMod> &mods() const { return mods_; };
    bool contains(const QString &id) const;
    //! Returns index of the given mod within the mods list, or -1 if not present.
    int modIndex(const QString &id) const;
    const CachedMod *mod(const QString &id, int *modIdx = nullptr) const;

    CachedMod *mod(const QString &id, int *modIdx = nullptr);

    CachedMod *addUnloaded(const SteamModInfo &steamInfo, OperationContext context, int *modIdx = nullptr);
    const CachedVersion *addZipVersion(const SteamModInfo &steamInfo, QIODevice &zipFile, QString *errorInfo = nullptr);
    const CachedVersion *addModVersion(const QString &modId, const QString &versionId, const QString &folderPath);
    void refresh(RefreshLevel = FULL);
    inline void save();

// file-visibility:
    ModCache *q;
    inline QString modPath(const QString &modId) const;
    inline QString modVersionPath(const QString &modId, const QString &versionId) const;

private:
    const ModManConfig &config_;
    //! All cached mods.
    QList<CachedMod> mods_;
    //! Index of mods by mod ID.
    QHash<QString, qsizetype> modIds_;

    void sortMods();
    void refreshIndex();
    const QHash<QString, QString> saveInstalledVersionIds() const;
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

    bool refresh(ModCache::RefreshLevel = ModCache::FULL, const QString &previousInstalledVersionId = QString());
    CachedVersion *refreshVersion(const QString &versionId, ModCache::RefreshLevel = ModCache::FULL, QString *errorInfo = nullptr);
    bool updateFromSteam(const SteamModInfo &steamInfo);
    const CachedVersion *markInstalledVersion(const QString &hash, const QString &expectedVersionId, bool *modified);
    inline void unmarkInstalled() { installedVersion_ = nullptr; };

    bool readDb(const QJsonObject &modObject);
    void writeDb(QJsonObject &modObject) const;

private:
    const ModCache::Impl &cache;
    QString id_;
    QList<CachedVersion> versions_;
    ModInfo info_;

    CachedVersion *installedVersion_;

    CachedVersion *findVersionByHash(const QString &hash, const QString &expectedVersionId);
    void sortVersions();
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

    const SpecMod asSpec() const;

    QString path() const;

// file-visibility:
    void setInstalled(bool value) { installed_ = value; };
    bool refresh(ModCache::RefreshLevel = ModCache::FULL, QString *errorInfo = nullptr) const;

private:
    const ModCache::Impl &cache;
    const QString modId;
    QString id_;
    mutable ModInfo info_;
    mutable std::optional<QDateTime> timestamp_;
    mutable std::optional<QString> version_;

    bool installed_;
    mutable QString hash_;
    mutable std::optional<SpecMod> specMod;
};

// Folder Structure: {cachePath}/workshop-{steamId}/{versionTime}/
// Time format is ISO8601, but with ':' replaced with '_' to be a valid folder name on Windows.
const QString formatVersionTime(const QDateTime &versionTime)
{
    return versionTime.toString(Qt::ISODate).replace(':', '_');
}

const QDateTime parseVersionTime(const QString &versionId)
{
    return QDateTime::fromString(QString(versionId).left(20).replace('_', ':'), Qt::ISODate);
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

bool compareVersionIds(const CachedVersion &a, const CachedVersion &b)
{
    return a.id() > b.id();
}

/**
 * @brief Fix extracted file names containing '\' separators that should be sub folders.
 * @param cacheDir The root directory of the cache. Used for log and error statements.
 * @param dir The directory into which files were extracted.
 * @param fileNames The absolute paths of the extracted files.
 *
 * The Mod Uploader creates non-conforming ZIP files using the local path separator, but QuaZip::QuaZip explicitly does not support such files.
 * In that case, on non-Windows systems, files were incorrectly extracted with "path\filename".
 */
bool fixFileNames(const QDir &cacheDir, const QDir &dir, const QStringList &filePaths, QString *errorInfo = nullptr)
{
    for (const QString &absolutePath : filePaths)
    {
        QFileInfo info(absolutePath);
        QString name = info.fileName();

        if (dir.absoluteFilePath(name) == absolutePath && name.contains('\\'))
        {
            QFile file(absolutePath);
            QString newPath = dir.absoluteFilePath(name.replace('\\', '/'));

            if (QDir().mkpath(QFileInfo(newPath).absolutePath()) && file.rename(newPath))
            {
                qCWarning(modcache).noquote() << QStringLiteral("Renamed %1 to %2").arg(cacheDir.relativeFilePath(absolutePath), cacheDir.relativeFilePath(newPath));
            }
            else
            {
                QString msg = QStringLiteral("Failed to rename %1 to %2").arg(cacheDir.relativeFilePath(absolutePath).toUtf8().constData(), cacheDir.relativeFilePath(newPath).toUtf8().constData());
                qCCritical(modcache).noquote() << msg;
                if (errorInfo)
                    *errorInfo = msg;
                return false;
            }
        }
    }
    return true;
}

ModCache::ModCache(const ModManConfig &config, QObject *parent)
    : QObject(parent), impl{std::make_unique<Impl>(config)}
{
    impl->q = this;
}

const QList<CachedMod> &ModCache::mods() const
{
    return impl->mods();
}

bool ModCache::contains(const QString &id) const
{
    return impl->contains(id);
}

int ModCache::modIndex(const QString &id) const
{
    return impl->modIndex(id);
}

const CachedMod *ModCache::mod(const QString &id) const
{
    return impl->mod(id);
}

const CachedMod *ModCache::addUnloaded(const SteamModInfo &steamInfo)
{
    return impl->addUnloaded(steamInfo, COMPLETE_OP);
}

const CachedVersion *ModCache::addZipVersion(const SteamModInfo &steamInfo, QIODevice &zipFile, QString *errorInfo)
{
    return impl->addZipVersion(steamInfo, zipFile, errorInfo);
}

const CachedVersion *ModCache::addModVersion(const QString &modId, const QString &versionId, const QString &folderPath)
{
    return impl->addModVersion(modId, versionId, folderPath);
}

void ModCache::refresh(ModCache::RefreshLevel level)
{
    impl->refresh(level);
}

const CachedVersion *ModCache::refreshVersion(const QString &modId, const QString &versionId, ModCache::RefreshLevel level)
{
    int modIdx;
    CachedMod *m = impl->mod(modId, &modIdx);
    if (!m)
        return nullptr;

    emit aboutToRefresh({modId}, {modIdx}, ModCache::VERSION_ONLY_HINT);
    const CachedVersion *v = m->impl()->refreshVersion(versionId, level);
    emit refreshed({modId}, {modIdx}, ModCache::VERSION_ONLY_HINT);
    return v;
}

void ModCache::saveMetadata()
{
    impl->save();
}

const CachedVersion *ModCache::markInstalledVersion(const QString &modId, const QString &hash, const QString &expectedVersionId)
{
    int modIdx;
    CachedMod *m = impl->mod(modId, &modIdx);
    if (m)
    {
        bool modified = false;
        const CachedVersion *v = m->impl()->markInstalledVersion(hash, expectedVersionId, &modified);
        if (modified)
            emit metadataChanged({modId}, {modIdx});
        return v;
    }
    return nullptr;
}

void ModCache::unmarkInstalledMod(const QString &modId)
{
    int modIdx;
    CachedMod *m = impl->mod(modId, &modIdx);
    if (m)
    {
        m->impl()->unmarkInstalled();
        emit metadataChanged({modId}, {modIdx});
    }
}

ModCache::~ModCache() = default;

ModCache::Impl::Impl(const ModManConfig &config)
    : config_(config)
{}

bool ModCache::Impl::contains(const QString &id) const
{
    return modIds_.contains(id);
}

int ModCache::Impl::modIndex(const QString &id) const
{
    if (modIds_.contains(id))
        return modIds_[id];
    return -1;
}

const CachedMod *ModCache::Impl::mod(const QString &id, int *modIdx) const
{
    if (modIds_.contains(id))
    {
        int idx = modIds_[id];
        if (modIdx)
            *modIdx = idx;
        return &mods_.at(idx);
    }
    return nullptr;
}

CachedMod *ModCache::Impl::mod(const QString &id, int *modIdx)
{
    if (modIds_.contains(id))
    {
        int idx = modIds_[id];
        if (modIdx)
            *modIdx = idx;
        return &mods_[modIds_[id]];
    }
    return nullptr;
}

CachedMod *ModCache::Impl::addUnloaded(const SteamModInfo &steamInfo, OperationContext context, int *modIdx)
{
    if (steamInfo.id.isEmpty())
        return nullptr;

    QString modId = steamInfo.modId();
    if (modIds_.contains(modId))
        return nullptr;

    CachedMod mod(*this, modId);
    if (mod.impl()->updateFromSteam(steamInfo))
    {
        emit q->aboutToAppendMods({mod.id()});
        int idx = mods_.size();
        if (modIdx)
            *modIdx = idx;
        modIds_[mod.id()] = idx;
        mods_.append(mod);
        if (context == COMPLETE_OP) // else, caller will emit the completion signal.
            emit q->appendedMods();
        return &mods_.last();
    }

    return nullptr;
}

const CachedVersion *ModCache::Impl::addZipVersion(const SteamModInfo &steamInfo, QIODevice &zipFile, QString *errorInfo)
{
    if (steamInfo.id.isEmpty())
        return nullptr;

    const QString modId = steamInfo.modId();
    const QString versionId = formatVersionTime(steamInfo.lastUpdated);
    const QDir cacheDir(config_.cachePath());
    QString outputPath = modVersionPath(modId, versionId);

    int modIdx;
    CachedMod *m = mod(modId, &modIdx);
    bool isNewMod = !m;
    if (isNewMod)
        m = addUnloaded(steamInfo, PARTIAL_OP, &modIdx);
    if (!m)
    {
        qCCritical(modcache).noquote() << "Couldn't add mod to cache " << modId;
        if (errorInfo)
            *errorInfo = "Failed adding new mod to cache";
        return nullptr;
    }

    if (!FileUtils::removeModDir(outputPath, errorInfo))
        return nullptr;

    qCDebug(modcache).noquote() << modId << "Unzip Start" << outputPath;
    QStringList files = JlCompress::extractDir(&zipFile, outputPath);
    bool ok = fixFileNames(cacheDir, QDir(outputPath), files, errorInfo);
    qCDebug(modcache).noquote() << modId << "Unzip End";
    if (!ok) return nullptr;

    if (!isNewMod)
        emit q->aboutToRefresh({modId}, {modIdx}, ModCache::VERSION_ONLY_HINT);
    const CachedVersion *v = m->impl()->refreshVersion(versionId, ModCache::FULL, errorInfo);
    if (isNewMod)
        emit q->appendedMods();
    else
        emit q->refreshed({modId}, {modIdx}, ModCache::VERSION_ONLY_HINT);
    return v;
}

const CachedVersion *ModCache::Impl::addModVersion(const QString &modId, const QString &versionId, const QString &folderPath)
{
    QDir sourceDir(folderPath);
    if (!sourceDir.exists("modinfo.txt"))
    {
        qCWarning(modcache).noquote() << modId << "No modinfo in source folder: " << folderPath;
        return nullptr;
    }

    QString outputPath = modVersionPath(modId, versionId);
    if (!FileUtils::removeModDir(outputPath))
        return nullptr;
    qCDebug(modcache) << "Copying" << folderPath << "to" << outputPath;
    if (!FileUtils::copyRecursively(folderPath, outputPath))
    {
        qCWarning(modcache).noquote() << modId << "Failed to copy" << folderPath << "to" << outputPath;
        return nullptr;
    }

    int modIdx;
    CachedMod *m = mod(modId, &modIdx);
    if (m)
    {
        emit q->aboutToRefresh({modId}, {modIdx}, ModCache::VERSION_ONLY_HINT);
        const CachedVersion *v = m->impl()->refreshVersion(versionId);
        emit q->refreshed({modId}, {modIdx}, ModCache::VERSION_ONLY_HINT);
        return v;
    }
    else
    {
        CachedMod newMod(*this, modId);
        const CachedVersion *v = newMod.impl()->refreshVersion(versionId);
        if (v)
        {
            emit q->aboutToAppendMods({modId});
            modIds_[modId] = mods_.size();
            mods_.append(newMod);
            emit q->appendedMods();
        }
        return v;
    }
}

const QHash<QString, QString> ModCache::Impl::saveInstalledVersionIds() const
{
    QHash<QString, QString> map;
    for (const auto &cm : mods())
    {
        const CachedVersion *iv = cm.impl()->installedVersion();
        if (iv)
            map.insert(cm.id(), iv->id());
    }

    return map;
}

void ModCache::Impl::refresh(RefreshLevel level)
{
    QDir cacheDir(config_.cachePath());
    qCDebug(modcache).noquote() << "cache:refresh() Start" << cacheDir.path();

    emit q->aboutToRefresh();

    const QHash<QString, QString> installedVersionIds = saveInstalledVersionIds();

    modIds_.clear();
    mods_.clear();
    readModManDb();
    sortMods();

    cacheDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    cacheDir.setSorting(QDir::Name);
    const QStringList modIds = cacheDir.entryList();
    mods_.reserve(mods_.size() + modIds.size());
    for (const auto &modId : modIds)
    {
        if (CachedMod *m = mod(modId))
        {
            m->impl()->refresh(level, installedVersionIds.value(modId));
        }
        else
        {
            CachedMod newMod(*this, modId);
            if (newMod.impl()->refresh(level, installedVersionIds.value(modId)))
                mods_.append(newMod);
        }
    }

    sortMods(); // Automatically refreshes the index.

    qCDebug(modcache).noquote().nospace() << "cache:refresh() End mods:" << mods_.size();
    emit q->refreshed();
}

void ModCache::Impl::save()
{
    emit q->aboutToRefresh(QStringList(), QList<int>(), ModCache::SORT_ONLY_HINT);
    sortMods();
    emit q->refreshed(QStringList(), QList<int>(), ModCache::SORT_ONLY_HINT);
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
        {
            if (cachedVersion.info().isEmpty())
                cachedVersion.impl()->refresh();
            return &cachedVersion;
        }
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

bool CachedMod::Impl::refresh(ModCache::RefreshLevel level, const QString &previousInstalledVersionId)
{
    QDir modDir(cache.modPath(id_));

    QString installedVersionId = installedVersion_ ? installedVersion_->id() : previousInstalledVersionId;

    modDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    modDir.setSorting(QDir::Name | QDir::Reversed);
    const QFileInfoList versionPaths = modDir.entryInfoList();
    versions_.clear();
    versions_.reserve(versionPaths.size());
    ModCache::RefreshLevel versionLevel = level == ModCache::LATEST_ONLY ? ModCache::FULL : level;
    for (const auto &versionPath : versionPaths)
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
        if (installedVersion_)
            installedVersion_->impl()->setInstalled(true);
        else
            qCWarning(modcache) << info_.toString() << "no longer contains installed version after refresh:" << installedVersionId;
    }

    if (versions_.size() > 0)
    {
        info_ = versions_.first().info();
        return true;
    }
    return false;
}

CachedVersion *CachedMod::Impl::refreshVersion(const QString &versionId, ModCache::RefreshLevel level, QString *errorInfo)
{
    if (CachedVersion *v = version(versionId))
    {
        if (v->impl()->refresh(level))
            return v;
    }
    else
    {
        CachedVersion newVersion(cache, id(), versionId);
        if (newVersion.impl()->refresh(level, errorInfo))
        {
            versions_.append(newVersion);
            sortVersions();
            if (versions_.first().id() == versionId)
                info_ = newVersion.info();
            return version(versionId);
        }
    }
    return nullptr;
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

const CachedVersion *CachedMod::Impl::markInstalledVersion(const QString &hash, const QString &expectedVersionId, bool *modified)
{
    CachedVersion *cachedVersion = findVersionByHash(hash, expectedVersionId);

    if (cachedVersion == installedVersion_)
    {
        if (modified)
            *modified = false;
        return cachedVersion;
    }
    if (modified)
        *modified = true;

    // Clear flags on any previously installed version.
    if (installedVersion_)
        installedVersion_->impl()->setInstalled(false);

    if (!cachedVersion)
        return nullptr;

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

CachedVersion *CachedMod::Impl::findVersionByHash(const QString &hash, const QString &expectedVersionId)
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

void CachedMod::Impl::sortVersions()
{
    QString installedVersionId = installedVersion_ ? installedVersion_->id() : QString();

    std::sort(versions_.begin(), versions_.end(), compareVersionIds);

    if (!installedVersionId.isEmpty())
    {
        installedVersion_ = version(installedVersionId);
        if (!installedVersion_)
            qCWarning(modcache) << info_.toString() << "no longer contains installed version after refresh:" << installedVersionId;
    }
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

const QString CachedVersion::toString(StringFormat format) const
{
    if (auto version = impl()->version())
    {
        if (format == FORMAT_FULL)
            return QStringLiteral("%1 [%2]").arg(*version, impl()->id());
        else
            return *version;
    }
    else
    {
        return QStringLiteral("[%2]").arg(impl()->id());
    }

}

const SpecMod CachedVersion::asSpec() const
{
    return impl()->asSpec();
}

QString CachedVersion::path() const
{
    return impl()->path();
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

const SpecMod CachedVersion::Impl::asSpec() const
{
    if (!specMod)
        specMod.emplace(modId, id_, info_.name(), info_.version());

    return *specMod;
}

QString CachedVersion::Impl::path() const
{
    return cache.modVersionPath(modId, id_);
}

bool CachedVersion::Impl::refresh(ModCache::RefreshLevel level, QString *errorInfo) const
{
    QDir modVersionDir(cache.modVersionPath(modId, id_));
    if (!modVersionDir.exists("modinfo.txt"))
    {
        qCDebug(modcache).noquote() << QString("modversion:refresh(%1,%2)").arg(modId, id_) << "skipped: No modinfo.txt";
        if (errorInfo)
            *errorInfo = QStringLiteral("No modinfo.txt");
        return false;
    }

    hash_.clear();
    specMod.reset();

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
