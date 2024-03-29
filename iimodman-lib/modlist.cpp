#include "fileutils.h"
#include "modcache.h"
#include "modinfo.h"
#include "modlist.h"
#include "modmanconfig.h"
#include "modspec.h"

#include <QDir>
#include <QJsonObject>
#include <QList>
#include <QLoggingCategory>
#include <optional>
#include <modsignature.h>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(modlist)
Q_LOGGING_CATEGORY(modlist, "modlist", QtWarningMsg);


//! Private implementation of ModList.
//! Additionally exposes methods to the InstalledMod children defined in this file.
class ModList::Impl
{
public:
    Impl(const ModManConfig &config, ModCache *cache);

    inline const QList<InstalledMod> &mods() const { return mods_; }
    bool contains(const QString &id) const;
    InstalledMod *mod(const QString &id);
    const InstalledMod *mod(const QString &id) const;

    void refresh(RefreshLevel level = FULL);
    const InstalledMod *installMod(const SpecMod &specMod, QString *errorInfo = nullptr);
    bool removeMod(const QString &modId, QString *errorInfo = nullptr);

// file-visibility:
    ModList *q;
    inline const ModManConfig &config() const { return config_; }
    inline const ModCache *cache() const { return cache_; }
    inline ModCache *cache() { return cache_; }
    QString modPath(const QString &installedId) const;

private:
    const ModManConfig &config_;
    ModCache *cache_;
    QList<InstalledMod> mods_;
    //! Index of mods by mod ID.
    QHash<QString, qsizetype> modIds_;

    void refreshIndex();
    const QHash<QString, QString> saveCacheVersionIds() const;
};

//! Private implementation of InstalledMod.
//! Additionally exposes mutators called by its parent ModList;
class InstalledMod::Impl
{
public:
    Impl(ModList::Impl &parent, const QString &id);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    inline const QString &alias() const { return alias_; };
    inline const QString &installedId() const { return alias_.isEmpty() ? id_ : alias_; }
    const QString &hash() const;

    const CachedVersion *cacheVersion() const;
    const CachedVersion *alternateCacheVersion(const QString &cacheId) const;
    const SpecMod asSpec() const;
    QString path() const;

    bool writeMetadataClaim(const QString &cacheId, const QString &cacheVersionId) const;

// file-visibility:
    inline const QString &cacheVersionId() const { return cacheVersionId_; }
    inline void overrideId(const QString &newId) { id_ = newId; }
    inline void setAlias(const QString &newAlias) { alias_ = newAlias; }
    bool refresh(ModList::RefreshLevel level = ModList::FULL, const QString &expectedCacheVersionId = QString(), ModInfo::IDStatus idStatus = ModInfo::ID_LOCKED);

private:
    ModList::Impl &parent_;
    QString id_;
    ModInfo info_;
    QString alias_;

    mutable QString cacheVersionId_;
    mutable QString hash_;
    mutable std::optional<SpecMod> specMod;

    inline const ModList::Impl &parent() const { return parent_; }
};


static bool writeMetadata(const QString &installedPath, const CachedMod *cm, const CachedVersion *cv)
{
    if (!cm)
        return false;

    QJsonObject root;
    root["modId"] = cm->id();
    if (cv)
        root["versionId"] = cv->id();

    QDir installedDir(installedPath);
    QString errorInfo;
    if (!FileUtils::writeJSON(installedDir.filePath("modman.json"), root, &errorInfo))
    {
        qCWarning(modlist).noquote() << "Failed to write modman.json to " << installedPath << ": " << errorInfo;
        return false;
    }
    return true;
}


ModList::ModList(const ModManConfig &config, ModCache *cache, QObject *parent)
    : QObject(parent), impl{std::make_unique<Impl>(config, cache)}
{
    impl->q = this;
}

const QList<InstalledMod> &ModList::mods() const
{
    return impl->mods();
}

bool ModList::contains(const QString &id) const
{
    return impl->contains(id);
}

const InstalledMod *ModList::mod(const QString &id) const
{
    return impl->mod(id);
}

void ModList::refresh(ModList::RefreshLevel level)
{
    impl->refresh(level);
}

const InstalledMod *ModList::installMod(const SpecMod &specMod, QString *errorInfo)
{
    return impl->installMod(specMod, errorInfo);
}

bool ModList::removeMod(const QString &modId, QString *errorInfo)
{
    return impl->removeMod(modId, errorInfo);
}

ModList::~ModList() = default;

ModList::Impl::Impl(const ModManConfig &config, ModCache *cache)
    : config_(config), cache_(cache)
{}

bool ModList::Impl::contains(const QString &id) const
{
    return modIds_.contains(id);
}

InstalledMod *ModList::Impl::mod(const QString &id)
{
    if (modIds_.contains(id))
        return &mods_[modIds_[id]];
    return nullptr;
}

const InstalledMod *ModList::Impl::mod(const QString &id) const
{
    if (modIds_.contains(id))
        return &mods_[modIds_[id]];
    return nullptr;
}

const QHash<QString, QString> ModList::Impl::saveCacheVersionIds() const
{
    QHash<QString, QString> map;
    for (const auto &im : mods())
    {
        const QString &cvId = im.impl()->cacheVersionId();
        // Index by installed ID, because that's what will be available first on the refresh.
        map.insert(im.impl()->installedId(), cvId);
    }

    return map;
}

void ModList::Impl::refresh(ModList::RefreshLevel level)
{
    QDir installDir(config_.modPath());
    qCDebug(modlist).noquote() << "installed:refresh() Start" << installDir.path();

    const QHash<QString, QString> cacheVersionIds = saveCacheVersionIds();

    emit q->aboutToRefresh();
    modIds_.clear();
    mods_.clear();

    if (!config_.hasValidPaths())
    {
        emit q->refreshed();
        return;
    }

    installDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    installDir.setSorting(QDir::Name);
    const QStringList modIds = installDir.entryList();
    mods_.reserve(modIds.size());
    for (const auto &modId : modIds)
    {
        InstalledMod mod(*this, modId);
        if (mod.impl()->refresh(level, cacheVersionIds.value(modId), ModInfo::ID_TENTATIVE))
            mods_.append(mod);
    }

    refreshIndex();

    // Unmark previously installed mods.
    for (auto it = cacheVersionIds.keyBegin(), endIt = cacheVersionIds.keyEnd(); it != endIt; ++it)
    {
        const QString &modId = *it;
        if (!modIds_.contains(modId))
            cache()->unmarkInstalledMod(modId);
    }

    emit q->refreshed();
}

const InstalledMod *ModList::Impl::installMod(const SpecMod &specMod, QString *errorInfo)
{
    if (!config_.hasValidPaths())
    {
        if (errorInfo)
            *errorInfo = QStringLiteral("No Invisible Inc. install found.");
        return nullptr;
    }

    const QString &modId = specMod.id();
    const QString &alias = specMod.alias();
    const bool useAlias = !alias.isEmpty();
    const CachedMod *cm = cache()->mod(modId);
    InstalledMod *im = mod(modId);
    bool useLatestVersion = specMod.versionId().isEmpty();

    if (specMod.versionId() == '-')
    {
        // Keep currently installed version, if possible.
        InstalledMod *im = mod(modId);
        if (im)
        {
            if (im->alias() != alias)
            {
                if (errorInfo)
                    *errorInfo = QStringLiteral("Cannot preserve existing version when alias doesn't match: %1 %2").arg(im->alias(), alias);
                qCWarning(modlist).noquote() << "Alias mismatch " << modId << " currently: " << im->alias() << " requested: " << alias;
                return nullptr;
            }
            im->impl()->refresh();
            return im;
        }
        else
            useLatestVersion = true;
    }

    if (!cm)
    {
        if (errorInfo)
            *errorInfo = QStringLiteral("Mod not in cache.");
        return nullptr;
    }
    const CachedVersion *cv = useLatestVersion ? cm->latestVersion() : cm->version(specMod.versionId());
    if (!cv)
    {
        if (errorInfo)
            *errorInfo = specMod.versionId().isEmpty() ? QStringLiteral("Mod has no cached versions.") : QStringLiteral("Mod version not in cache: %1").arg(specMod.versionId());
        return nullptr;
    }
    const QString inputPath = cv->path();
    const QString outputPath = modPath(useAlias ? alias : modId);
    if (!FileUtils::removeModDir(outputPath, errorInfo))
        return nullptr;
    if (im && im->alias() != alias)
    {
        // Also uninstall the existing install of this mod with a different folder.
        const QString aliasPath = modPath(im->impl()->installedId());
        if (!FileUtils::removeModDir(aliasPath, errorInfo))
            return nullptr;
    }
    qCDebug(modlist) << "Copying" << inputPath << "to" << outputPath;
    if (!FileUtils::copyRecursively(inputPath, outputPath, errorInfo))
    {
        qCWarning(modlist).noquote() << "Failed to copy" << inputPath << "to" << outputPath;
        return nullptr;
    }

    bool writeOk = writeMetadata(outputPath, cm, cv);
    Q_UNUSED(writeOk); // Continue even on failure. The metadata isn't critical.

    if (im)
    {
        im->impl()->setAlias(alias);
        if (im->impl()->refresh(FULL, cv->id()))
            return im;
    }
    else
    {
        InstalledMod newMod(*this, modId);
        newMod.impl()->setAlias(alias);
        if (newMod.impl()->refresh(FULL, cv->id()))
        {
            modIds_[modId] = mods_.size();
            mods_.append(newMod);
            return &mods_.last();
        }
    }
    return nullptr;
}

bool ModList::Impl::removeMod(const QString &modId, QString *errorInfo)
{
    if (!config_.hasValidPaths())
    {
        if (errorInfo)
            *errorInfo = QStringLiteral("No Invisible Inc. install found.");
        return false;
    }

    InstalledMod *im = mod(modId);
    if (im)
    {
        const QString outputPath = modPath(im->impl()->installedId());
        if (!FileUtils::removeModDir(outputPath, errorInfo))
            return false;
    }
    else if (errorInfo)
        *errorInfo = QStringLiteral("Mod not installed.");

    return im;
}

QString ModList::Impl::modPath(const QString &installedId) const
{
    if (installedId.isEmpty()) // Ensure we never accidentally try to "remove" the entire mod directory.
        qFatal("Cannot process a mod with empty path.");
    QDir installDir(config_.modPath());
    return installDir.absoluteFilePath(installedId);
}

void ModList::Impl::refreshIndex()
{
    modIds_.clear();
    for (qsizetype i = 0; i < mods_.size(); ++i)
    {
        const InstalledMod &mod = mods_.at(i);
        QString modId = mod.id();
        if (!mod.alias().isEmpty())
            while (modIds_.contains(modId))
            {
                // Conflict between folders claiming the same ID!
                // This mod has an alias, so can move out of the way.
                modId = modId + "-1";
                mods_[i].impl()->overrideId(modId);
            }
        else if (modIds_.contains(modId))
        {
            // Conflict between folders claiming the same ID!
            // This mod does not have an alias, so move the other mod.
            InstalledMod &other = mods_[modIds_[modId]];
            QString otherId = modId;
            do
            {
                otherId = otherId + "-1";
                other.impl()->overrideId(otherId);
            }
            while (modIds_.contains(otherId));
        }
        modIds_[modId] = i;
    }
}

InstalledMod::InstalledMod(ModList::Impl &parent, const QString &id)
    : impl_{std::make_shared<Impl>(parent, id)}
{}

const QString &InstalledMod::id() const
{
    return impl()->id();
}

const ModInfo &InstalledMod::info() const
{
    return impl()->info();
}

const QString &InstalledMod::alias() const
{
    return impl()->alias();
}

const QString &InstalledMod::hash() const
{
    return impl()->hash();
}

bool InstalledMod::hasCacheVersion() const
{
    return impl()->cacheVersion();
}

const CachedVersion *InstalledMod::cacheVersion() const
{
    return impl()->cacheVersion();
}

const CachedVersion *InstalledMod::alternateCacheVersion(const QString &cacheId) const
{
    return impl()->alternateCacheVersion(cacheId);
}

const SpecMod InstalledMod::asSpec() const
{
    return impl()->asSpec();
}

QString InstalledMod::path() const
{
    return impl()->path();
}

QString InstalledMod::versionString() const
{
    const CachedVersion *cachedVersion = cacheVersion();
    if (cachedVersion)
        return cachedVersion->toString();
    else
        return impl()->info().version();
    return QString();
}

bool InstalledMod::writeMetadataClaim(const QString &cacheId, const QString &cacheVersionId) const
{
    return impl()->writeMetadataClaim(cacheId, cacheVersionId);
}


InstalledMod::Impl::Impl(ModList::Impl &parent, const QString &id)
    : parent_(parent), id_(id)
{}

const QString &InstalledMod::Impl::hash() const
{
    if (hash_.isEmpty())
        hash_ = ModSignature::hashModPath(parent().modPath(installedId()));
    return hash_;
}

const CachedVersion *InstalledMod::Impl::cacheVersion() const
{
    if (cacheVersionId_.isEmpty())
        return nullptr;

    const auto *cm = parent().cache()->mod(id_);
    if (!cm)
        return nullptr;

    const auto *cv = cm->versionFromHash(hash(), cacheVersionId_);
    if (cv)
        cacheVersionId_ = cv->id();
    else
        cacheVersionId_.clear();
    return cv;
}

const CachedVersion *InstalledMod::Impl::alternateCacheVersion(const QString &cacheId) const
{
    const auto *cm = parent().cache()->mod(cacheId);
    return cm ? cm->versionFromHash(hash(), cacheVersionId_) : nullptr;
}

const SpecMod InstalledMod::Impl::asSpec() const
{
    if (!specMod)
    {
        const CachedVersion *v = cacheVersion();
        specMod.emplace(id_, v ? v->id() : QStringLiteral("-"), alias_, info_.name(), info_.version());
    }

    return *specMod;
}

QString InstalledMod::Impl::path() const
{
    return parent().modPath(installedId());
}

bool InstalledMod::Impl::writeMetadataClaim(const QString &cacheId, const QString &cacheVersionId) const
{
    if (!parent().config().hasValidPaths())
        return false;

    const CachedMod *cm = parent().cache()->mod(cacheId);
    if (!cm)
        return false;

    const CachedVersion *cv = cm->versionFromHash(hash(), cacheVersionId.isEmpty() ? cacheVersionId_ : cacheVersionId);
    const QString installedPath = parent().modPath(installedId());
    return writeMetadata(installedPath, cm, cv);
}

bool InstalledMod::Impl::refresh(ModList::RefreshLevel level, const QString &expectedCacheVersionId, ModInfo::IDStatus idStatus)
{
    QDir modDir(parent().modPath(installedId()));
    if (!modDir.exists("modinfo.txt"))
    {
        qCDebug(modlist).noquote() << QString("installedmod:refresh(%1)").arg(id_) << "skipped: No modinfo.txt";
        return false;
    }
    qCDebug(modlist).noquote().nospace() << QString("installedmod:refresh(%1)").arg(id_);

    hash_.clear();
    specMod.reset();

    const QJsonObject savedMetadata = FileUtils::readJSON(modDir.filePath("modman.json"));
    if (savedMetadata.contains("modId") && savedMetadata["modId"].isString())
    {
        QString modId = savedMetadata["modId"].toString();
        if (modId != id_)
        {
            alias_ = id_;
            id_ = modId;
            idStatus = ModInfo::ID_LOCKED; // Treat modman.json as truth.
        }
    }
    // Prefer to use an in-memory recognized version ID. Will be refreshed below when marking installed version.
    if (cacheVersionId_.isEmpty() && savedMetadata.contains("versionId") && savedMetadata["versionId"].isString())
        cacheVersionId_ = savedMetadata["versionId"].toString();

    if (level == ModList::ID_ONLY)
        return true;

    QFile infoFile = QFile(modDir.filePath("modinfo.txt"));
    info_ = ModInfo::readModInfo(infoFile, id_, idStatus);
    infoFile.close();
    if (idStatus == ModInfo::ID_TENTATIVE && id_ != info_.id())
    {
        alias_ = id_;
        id_ = info_.id();
    }

    if (level == ModList::CONTENT_ONLY)
        return true;

    ModCache *cache = parent_.cache();
    assert(cache);
    if (cache->contains(id_))
    {
        hash_ = ModSignature::hashModPath(modDir.path());
        const CachedVersion *version = cache->markInstalledVersion(
                id_, hash_,
                expectedCacheVersionId.isNull() ? cacheVersionId_ : expectedCacheVersionId);
        if (version)
            cacheVersionId_ = version->id();
        else
            cacheVersionId_.clear();
    }
    else
        cacheVersionId_.clear();


    return true;
}

}  // namespace iimodmanager
