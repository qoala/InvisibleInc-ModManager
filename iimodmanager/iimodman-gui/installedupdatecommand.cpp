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
            if (!installMod(cv->asSpec(), cm))
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

bool InstalledUpdateCommand::installMod(SpecMod specMod, const CachedMod *from)
{
    QString fromVersion = from->info().version();

    QString errorInfo;
    const InstalledMod *installed = app.modList().installMod(specMod, &errorInfo);
    if (installed)
    {
        emit textOutput(QStringLiteral("  %1 installed \t%2 => %3").arg(
                    installed->info().toString(),
                    util::displayVersion(fromVersion),
                    util::displayVersion(installed->info().version())));
        return true;
    }
    else
    {
        const CachedMod *cm = app.cache().mod(specMod.id());
        emit textOutput(QStringLiteral("  Failed to install %1: %2").arg(cm ? cm->info().toString() : specMod.id(), errorInfo));
        return false;
    }
}

} // namespace iimodmanager
