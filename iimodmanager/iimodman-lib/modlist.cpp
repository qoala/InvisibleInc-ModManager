#include "fileutils.h"
#include "modcache.h"
#include "modinfo.h"
#include "modlist.h"
#include "modmanconfig.h"
#include "modspec.h"

#include <QDir>
#include <QList>
#include <QLoggingCategory>
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

    inline const QList<InstalledMod> mods() const { return mods_; }
    InstalledMod *mod(const QString &id);
    const InstalledMod *mod(const QString &id) const;

    void refresh(RefreshLevel level = FULL);
    const InstalledMod *installMod(const SpecMod &specMod);
    const InstalledMod *removeMod(const QString &modId);

// file-visibility:
    inline ModCache *cache() const { return cache_; }
    QString modPath(const QString &modId) const;

private:
    const ModManConfig &config_;
    ModCache *cache_;
    QList<InstalledMod> mods_;
    //! Index of mods by mod ID.
    QMap<QString, qsizetype> modIds_;

    void refreshIndex();
};

//! Private implementation of InstalledMod.
//! Additionally exposes mutators called by its parent ModList;
class InstalledMod::Impl
{
public:
    Impl(const ModList::Impl &parent, const QString &id);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };
    const QString &hash() const;

    bool hasCacheVersion() const;
    const CachedVersion *cacheVersion() const;
    const SpecMod asSpec() const;
    QString path() const;

// file-visibility:
    bool refresh(ModList::RefreshLevel level = ModList::FULL);

private:
    const ModList::Impl &parent;
    QString id_;
    ModInfo info_;
    QString cacheVersionId_;

    mutable QString hash_;
    mutable std::optional<SpecMod> specMod;
};

ModList::ModList(const ModManConfig &config, ModCache *cache, QObject *parent)
    : QObject(parent), impl{std::make_unique<Impl>(config, cache)}
{}

const QList<InstalledMod> ModList::mods() const
{
    return impl->mods();
}

const InstalledMod *ModList::mod(const QString &id) const
{
    return impl->mod(id);
}

void ModList::refresh(ModList::RefreshLevel level)
{
    impl->refresh(level);
}

const InstalledMod *ModList::installMod(const SpecMod &specMod)
{
    return impl->installMod(specMod);
}

const InstalledMod *ModList::removeMod(const QString &modId)
{
    return impl->removeMod(modId);
}

ModList::~ModList() = default;

ModList::Impl::Impl(const ModManConfig &config, ModCache *cache)
    : config_(config), cache_(cache)
{}

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

void ModList::Impl::refresh(ModList::RefreshLevel level)
{
    QDir installDir(config_.modPath());
    qCDebug(modlist).noquote() << "installed:refresh() Start" << installDir.path();

    installDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    installDir.setSorting(QDir::Name);
    const QStringList modIds = installDir.entryList();
    mods_.clear();
    mods_.reserve(modIds.size());
    for (auto modId : modIds)
    {
        InstalledMod mod(*this, modId);
        if (mod.impl()->refresh(level))
            mods_.append(mod);
    }

    refreshIndex();
}

const InstalledMod *ModList::Impl::installMod(const SpecMod &specMod)
{
    const QString &modId = specMod.id();
    const CachedMod *cm = cache()->mod(modId);
    if (!cm)
    {
        return nullptr;
    }
    const CachedVersion *cv = specMod.versionId().isEmpty() ? cm->latestVersion() : cm->version(specMod.versionId());
    if (!cv)
    {
        return nullptr;
    }
    const QString inputPath = cv->path();
    const QString outputPath = modPath(modId);
    FileUtils::removeModDir(outputPath);
    qCDebug(modlist) << "Copying" << inputPath << "to" << outputPath;
    if (!FileUtils::copyRecursively(inputPath, outputPath))
    {
        qCWarning(modlist).noquote() << "Failed to copy" << inputPath << "to" << outputPath;
        return nullptr;
    }

    InstalledMod *im = mod(modId);
    if (im)
    {
        if (im->impl()->refresh())
            return im;
    }
    else
    {
        InstalledMod newMod(*this, modId);
        if (newMod.impl()->refresh())
        {
            modIds_[modId] = mods_.size();
            mods_.append(newMod);
            return &mods_.last();
        }
    }
    return nullptr;
}

const InstalledMod *ModList::Impl::removeMod(const QString &modId)
{
    InstalledMod *im = mod(modId);
    if (im)
    {
        const QString outputPath = modPath(modId);
        FileUtils::removeModDir(outputPath);
    }
    return im;
}

QString ModList::Impl::modPath(const QString &modId) const
{
    QDir installDir(config_.modPath());
    return installDir.absoluteFilePath(modId);
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

InstalledMod::InstalledMod(const ModList::Impl &parent, const QString &id)
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

InstalledMod::Impl::Impl(const ModList::Impl &parent, const QString &id)
    : parent(parent), id_(id)
{}

const QString &InstalledMod::Impl::hash() const
{
    if (hash_.isEmpty())
        hash_ = ModSignature::hashModPath(parent.modPath(id_));
    return hash_;
}

bool InstalledMod::Impl::hasCacheVersion() const
{
    if (auto cachedMod = parent.cache()->mod(id_))
        return cachedMod->installedVersion();
    return false;
}

const CachedVersion *InstalledMod::Impl::cacheVersion() const
{
    if (auto cachedMod = parent.cache()->mod(id_))
        return cachedMod->installedVersion();
    return nullptr;
}

const SpecMod InstalledMod::Impl::asSpec() const
{
    if (!specMod)
    {
        const CachedVersion *v = cacheVersion();
        specMod.emplace(id_, v ? v->id() : QString(), info_.name(), info_.version());
    }

    return *specMod;
}

QString InstalledMod::Impl::path() const
{
    return parent.modPath(id_);
}

bool InstalledMod::Impl::refresh(ModList::RefreshLevel level)
{
    QDir modDir(parent.modPath(id_));
    if (!modDir.exists("modinfo.txt"))
    {
        qCDebug(modlist).noquote() << QString("installedmod:refresh(%1)").arg(id_) << "skipped: No modinfo.txt";
        return false;
    }
    qCDebug(modlist).noquote().nospace() << QString("installedmod:refresh(%1)").arg(id_);

    hash_.clear();
    specMod.reset();

    if (level == ModList::ID_ONLY)
    {
        return true;
    }

    QFile infoFile = QFile(modDir.filePath("modinfo.txt"));
    info_ = ModInfo::readModInfo(infoFile, id_);
    infoFile.close();

    if (level == ModList::CONTENT_ONLY)
    {
        return true;
    }

    ModCache *cache = parent.cache();
    assert(cache);
    if (cache->contains(id_))
    {
        hash_ = ModSignature::hashModPath(modDir.path());
        const CachedVersion *version = cache->markInstalledVersion(id_, hash_);
        if (version)
            cacheVersionId_ = version->id();
    }

    return true;
}

}  // namespace iimodmanager
