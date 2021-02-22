#include "mod.h"
#include "modlist.h"

#include <QDir>

namespace iimodmanager {

ModList::ModList(const std::list<Mod> &mods, ModLocation location)
    : location_(location), mods_(mods)
{}

ModList ModList::readCurrent(const ModManConfig &config, ModLocation location)
{
    std::list<Mod> mods;

    QDir modsDir(location == CACHED ? config.cachePath() : config.installPath());
    assert(modsDir.exists());

    modsDir.setFilter(QDir::Dirs);
    modsDir.setSorting(QDir::Name);
    const QFileInfoList list = modsDir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QDir modDir(list.at(i).filePath());
        QFileInfo modInfo(modDir, "modinfo.txt");
        if (modInfo.isFile()) {

            mods.push_back(Mod::readModInfo(config, modDir.dirName(), location));
        }
    }

    return ModList(mods, location);
}

}  // namespace iimodmanager
