#include "installedstatuscommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

InstalledStatusCommand::InstalledStatusCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app)
{}

void InstalledStatusCommand::execute()
{
    app.refreshMods();

    emit textOutput("Currently Installed Mods\n(name)\t(id)\t(version)");

    QList<InstalledMod> mods(app.modList().mods());
    std::sort(mods.begin(), mods.end(), util::compareInstalledModNames);
    for (auto mod : mods) {
        const ModInfo &info = mod.info();

        QString line = QString("%1\t%2\t%3").arg(info.name()).arg(info.id()).arg(mod.versionString());
        if (!mod.hasCacheVersion())
        {
            line.append("\t(not in cache)");
        }
        else if (!app.cache().mod(mod.id())->latestVersion()->installed())
        {
            line.append("\t(update available)");
        }
        emit textOutput(line);
    }

    emit finished();
    deleteLater();
}

} // namespace iimodmanager
