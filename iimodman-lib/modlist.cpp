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

    bool hasCacheVersion() const;
    const CachedVersion *cacheVersion() const;
    const SpecMod asSpec() const;
    QString path() const;

// file-visibility:
    inline const QString &cacheVersionId() const { return cacheVersionId_; }
    inline void setAlias(const QString &newAlias) { alias_ = newAlias; }
    bool refresh(ModList::RefreshLevel level = ModList::FULL, const QString &expectedCacheVersionId = QString());

private:
    ModList::Impl &parent_;
    QString id_;
    ModInfo info_;
    QString alias_;
    QString cacheVersionId_;

    mutable QString hash_;
    mutable std::optional<SpecMod> specMod;

    inline const ModList::Impl &parent() const { return parent_; }
};

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
        map.insert(im.id(), cvId);
    }

    return map;
}

void ModList::Impl::refresh(ModList::RefreshLevel level)
{
    // TODO: enforce unique mod IDs.
    QDir installDir(config_.modPath());
    qCDebug(modlist).noquote() << "installed:refresh() Start" << installDir.path();

    const QHash<QString, QString> cacheVersionIds = saveCacheVersionIds();

    emit q->aboutToRefresh();


    installDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    installDir.setSorting(QDir::Name);
    const QStringList modIds = installDir.entryList();
    modIds_.clear();
    mods_.clear();
    mods_.reserve(modIds.size());
    for (const auto &modId : modIds)
    {
        InstalledMod mod(*this, modId);
        if (mod.impl()->refresh(level, cacheVersionIds.value(modId)))
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
        const QString aliasPath = modPath(im->alias());
        if (!FileUtils::removeModDir(aliasPath, errorInfo))
            return nullptr;
    }
    qCDebug(modlist) << "Copying" << inputPath << "to" << outputPath;
    if (!FileUtils::copyRecursively(inputPath, outputPath, errorInfo))
    {
        qCWarning(modlist).noquote() << "Failed to copy" << inputPath << "to" << outputPath;
        return nullptr;
    }

    QJsonObject root;
    root["modId"] = modId;
    root["versionId"] = cv->id();
    QDir outputDir(outputPath);
    QString sideErrorInfo;
    if (!FileUtils::writeJSON(outputDir.filePath("modman.json"), root, &sideErrorInfo))
    {
        qCWarning(modlist).noquote() << "Failed to write modman.json to " << outputPath << ": " << sideErrorInfo;
        // Continue even on failure. The metadata isn't critical.
    }

    if (im)
    {
        if (useAlias)
            im->impl()->setAlias(alias);
        if (im->impl()->refresh(FULL, cv->id()))
            return im;
    }
    else
    {
        InstalledMod newMod(*this, modId);
        if (useAlias)
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
    InstalledMod *im = mod(modId);
    if (im)
    {
        const QString outputPath = modPath(im->impl()->installedId());
        if (!FileUtils::removeModDir(outputPath, errorInfo))
            return false;
    }
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
        const QString &modId = mod.id();
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
    return impl()->hasCacheVersion();
}

const CachedVersion *InstalledMod::cacheVersion() const
{
    return impl()->cacheVersion();
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

InstalledMod::Impl::Impl(ModList::Impl &parent, const QString &id)
    : parent_(parent), id_(id)
{}

const QString &InstalledMod::Impl::hash() const
{
    if (hash_.isEmpty())
        hash_ = ModSignature::hashModPath(parent().modPath(installedId()));
    return hash_;
}

bool InstalledMod::Impl::hasCacheVersion() const
{
    if (const auto cachedMod = parent().cache()->mod(id_))
        return cachedMod->installedVersion();
    return false;
}

const CachedVersion *InstalledMod::Impl::cacheVersion() const
{
    if (const auto cachedMod = parent().cache()->mod(id_))
        return cachedMod->installedVersion();
    return nullptr;
}

const SpecMod InstalledMod::Impl::asSpec() const
{
    if (!specMod)
    {
        const CachedVersion *v = cacheVersion();
        specMod.emplace(id_, v ? v->id() : QStringLiteral("-"), info_.name(), info_.version());
    }

    return *specMod;
}

QString InstalledMod::Impl::path() const
{
    return parent().modPath(installedId());
}

bool InstalledMod::Impl::refresh(ModList::RefreshLevel level, const QString &expectedCacheVersionId)
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
        }
    }
    // Prefer to use an in-memory recognized version ID. Will be refreshed below when marking installed version.
    if (cacheVersionId_.isEmpty() && savedMetadata.contains("versionId") && savedMetadata["versionId"].isString())
        cacheVersionId_ = savedMetadata["versionId"].toString();

    if (level == ModList::ID_ONLY)
        return true;

    QFile infoFile = QFile(modDir.filePath("modinfo.txt"));
    info_ = ModInfo::readModInfo(infoFile, id_);
    infoFile.close();

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
