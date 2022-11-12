#include "cachestatuscommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <modinfo.h>

namespace iimodmanager {

CacheStatusCommand::CacheStatusCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app)
{}

void CacheStatusCommand::execute()
{
    app.refreshMods();

    emit textOutput("Cached Mods\n(name)\t(id)\t(version)");

    QList<CachedMod> mods(app.cache().mods());
    std::sort(mods.begin(), mods.end(), util::compareCachedModNames);
    for (auto mod : mods) {
        const ModInfo &info = mod.info();

        auto latestVersion = mod.latestVersion();
        auto installedVersion = mod.installedVersion();
        QString versionString = latestVersion ? latestVersion->toString() : "";
        QString line = QString("%1\t%2\t%3").arg(info.name()).arg(info.id()).arg(versionString);
        if (!latestVersion)
        {
            line.append("\t(no version downloaded)");
        }
        else if (latestVersion->installed())
        {
            line.append("\t(installed)");
        }
        else if (installedVersion)
        {
            line.append(QString("\t(installed: %1)").arg(installedVersion->toString()));
        }
        emit textOutput(line);
    }

    emit finished();
    deleteLater();
}

} // namespace iimodmanager
