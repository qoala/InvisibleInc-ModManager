#include "modlist.h"
#include "modinfo.h"

#include <QDir>
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

    void refresh(RefreshLevel level = FULL);

// file-visibility:
    inline ModCache *cache() const { return cache_; }
    QString modPath(const QString &modId) const;

private:
    const ModManConfig &config_;
    ModCache *cache_;
    QList<InstalledMod> mods_;
};

//! Private implementation of InstalledMod.
//! Additionally exposes mutators called by its parent ModList;
class InstalledMod::Impl
{
public:
    Impl(const ModList::Impl &parent, const QString &id);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };

    bool hasCacheVersion() const;
    const CachedVersion *cacheVersion() const;

// file-visibility:
    bool refresh(ModList::RefreshLevel level = ModList::FULL);

private:
    const ModList::Impl &parent;
    QString id_;
    ModInfo info_;
    QString hash_;
    QString cacheVersionId_;
};

ModList::ModList(const ModManConfig &config, ModCache *cache, QObject *parent)
    : QObject(parent), impl{std::make_unique<Impl>(config, cache)}
{}

const QList<InstalledMod> ModList::mods() const
{
    return impl->mods();
}

void ModList::refresh(ModList::RefreshLevel level)
{
    impl->refresh(level);
}

ModList::~ModList() = default;

ModList::Impl::Impl(const ModManConfig &config, ModCache *cache)
    : config_(config), cache_(cache)
{}

void ModList::Impl::refresh(ModList::RefreshLevel level)
{
    QDir installDir(config_.installPath());
    qCDebug(modlist).noquote() << "installed:refresh() Start" << installDir.path();

    installDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    installDir.setSorting(QDir::Name);
    QStringList modIds = installDir.entryList();
    mods_.clear();
    mods_.reserve(modIds.size());
    for (auto modId : modIds)
    {
        InstalledMod &mod = mods_.emplaceBack(*this, modId);
        if (!mod.impl()->refresh(level))
            mods_.removeLast();
    }
}

QString ModList::Impl::modPath(const QString &modId) const
{
    QDir installDir(config_.installPath());
    return installDir.absoluteFilePath(modId);
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

bool InstalledMod::hasCacheVersion() const
{
    return impl()->hasCacheVersion();
}

const CachedVersion *InstalledMod::cacheVersion() const
{
    return impl()->cacheVersion();
}

QString InstalledMod::versionString() const
{
    const CachedVersion *cachedVersion = cacheVersion();
    if (cachedVersion)
    {
        return cachedVersion->toString();
    }
    else
    {
        const QString &version = impl()->info().version();
        if (!version.isEmpty())
            return "v" + version;
    }
    return QString();
}

InstalledMod::Impl::Impl(const ModList::Impl &parent, const QString &id)
    : parent(parent), id_(id)
{}

bool InstalledMod::Impl::hasCacheVersion() const
{
    return !cacheVersionId_.isEmpty();
}

const CachedVersion *InstalledMod::Impl::cacheVersion() const
{
    if (!cacheVersionId_.isEmpty())
        return parent.cache()->mod(id_)->version(cacheVersionId_);
    return nullptr;
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
