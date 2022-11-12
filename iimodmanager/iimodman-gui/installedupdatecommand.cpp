#include "installedupdatecommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

InstalledUpdateCommand::InstalledUpdateCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app)
{}

void InstalledUpdateCommand::execute()
{
    app.refreshMods();

    emit textOutput("Checking cache for downloaded updates...");

    bool ok = true;
    for (const InstalledMod &im : app.modList().mods())
    {
        const CachedMod *cm = app.cache().mod(im.id());
        const CachedVersion *cv = cm ? cm->latestVersion() : nullptr;
        if (cv && !cv->installed())
            if (!installMod(cv->asSpec()))
            {
                ok = false;
                break;
            }
    }

    if (ok)
        emit textOutput("All mods up to date.");
    else
        emit textOutput("Update aborted.");
    emit finished();
    deleteLater();
}

bool InstalledUpdateCommand::installMod(SpecMod specMod)
{
    QString errorInfo;
    const InstalledMod *installed = app.modList().installMod(specMod, &errorInfo);
    if (installed)
    {
        emit textOutput(QString("  %1 installed %2").arg(installed->info().toString()).arg(installed->info().version()));
        return true;
    }
    else
    {
        const CachedMod *cm = app.cache().mod(specMod.id());
        emit textOutput(QString("  Failed to install %1: %2").arg(cm ? cm->info().toString() : specMod.id(), errorInfo));
        return false;
    }
}

} // namespace iimodmanager
