#include "modlist.h"
#include "modinfo.h"

#include <QDir>
#include <QLoggingCategory>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(modlist)
Q_LOGGING_CATEGORY(modlist, "modlist", QtWarningMsg);

ModList::ModList(const ModManConfig &config, QObject *parent)
    : QObject(parent), config_(config)
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

bool InstalledMod::refresh(ModList::RefreshLevel level)
{
    QDir modVersionDir(parent.modPath(id_));
    if (!modVersionDir.exists("modinfo.txt"))
    {
        qCDebug(modlist).noquote() << QString("installedmod:refresh(%1)").arg(id_) << "skipped: No modinfo.txt";
        return false;
    }

    if (level == ModList::ID_ONLY)
        return true;

    QFile infoFile = QFile(modVersionDir.filePath("modinfo.txt"));
    info_ = ModInfo::readModInfo(infoFile, id_);
    infoFile.close();

    qCDebug(modlist).noquote().nospace() << QString("installedmod:refresh(%1)").arg(id_) << " version=" << info_.version();
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
