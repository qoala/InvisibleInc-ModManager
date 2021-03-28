#include "modlist.h"
#include "modinfo.h"

#include <QDir>
#include <QLoggingCategory>
#include <modsignature.h>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(modlist)
Q_LOGGING_CATEGORY(modlist, "modlist", QtWarningMsg);

ModList::ModList(const ModManConfig &config, ModCache *cache, QObject *parent)
    : QObject(parent), config_(config), cache_(cache)
{}

QString ModList::modPath(const ModManConfig &config, const QString &modId)
{
    QDir installDir(config.installPath());
    return installDir.absoluteFilePath(modId);
}

void ModList::refresh(ModList::RefreshLevel level)
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
        if (!mod.refresh(level))
            mods_.removeLast();
    }
}

InstalledMod::InstalledMod(const ModList &parent, const QString &id)
    : parent(parent), id_(id)
{}

const CachedVersion *InstalledMod::cacheVersion() const
{
    if (!cacheVersionId_.isEmpty())
        return parent.cache()->mod(id_)->version(cacheVersionId_);
    return nullptr;
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
        const QString &version = info_.version();
        if (!version.isEmpty())
            return "v" + version;
    }
    return QString();
}

bool InstalledMod::refresh(ModList::RefreshLevel level)
{
    QDir modDir(parent.modPath(id_));
    if (!modDir.exists("modinfo.txt"))
    {
        qCDebug(modlist).noquote() << QString("installedmod:refresh(%1)").arg(id_) << "skipped: No modinfo.txt";
        return false;
    }

    if (level == ModList::ID_ONLY)
    {
        qCDebug(modlist).noquote().nospace() << QString("installedmod:refresh(%1)").arg(id_);
        return true;
    }

    QFile infoFile = QFile(modDir.filePath("modinfo.txt"));
    info_ = ModInfo::readModInfo(infoFile, id_);
    infoFile.close();

    if (level == ModList::CONTENT_ONLY)
    {
        qCDebug(modlist).noquote().nospace() << QString("installedmod:refresh(%1)").arg(id_) << " version=" << versionString();
        return true;
    }

    assert(parent.cache());
    const CachedMod *cachedMod = parent.cache()->mod(id_);
    if (cachedMod)
    {
        hash_ = ModSignature::hashModPath(modDir.path());
        for (auto version : cachedMod->versions())
        {
            if (version.hash() == hash_)
            {
                cacheVersionId_ = version.id();
                break;
            }
        }
    }

    qCDebug(modlist).noquote().nospace() << QString("installedmod:refresh(%1)").arg(id_) << " version=" << versionString();
    return true;
}

InstalledMod &InstalledMod::operator =(const InstalledMod &o)
{
   assert(&parent == &o.parent);
   id_ = o.id_;
   info_ = o.info_;
   return *this;
}

}  // namespace iimodmanager
