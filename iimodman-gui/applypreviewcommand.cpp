#include "applypreviewcommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <QFileDialog>
#include <QTimer>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

ApplyPreviewCommand::ApplyPreviewCommand(ModManGuiApplication  &app, ModSpecPreviewModel *preview, QObject *parent)
  : QObject(parent), app(app), preview(preview)
{}

void ApplyPreviewCommand::execute()
{
    if (!preview)
    {
        emit finished();
        deleteLater();
        return;
    }

    app.refreshMods();

    if (doSync())
        emit textOutput("Sync complete");
    else
        emit textOutput("Sync aborted");

    app.refreshMods();

    // Wait for any refresh callbacks to propagate.
    QTimer::singleShot(0, this, &ApplyPreviewCommand::finish);
}

void ApplyPreviewCommand::finish()
{
    // Cleanup the preview's state if it applied cleanly.
    if (!preview->canApply())
        preview->revert();

    emit finished();
    deleteLater();
}

bool ApplyPreviewCommand::doSync()
{
    QList<SpecMod> toAddMods;
    QList<SpecMod> toUpdateMods;
    QList<InstalledMod> toRemoveMods;
    preview->prepareChanges(&toAddMods, &toUpdateMods, &toRemoveMods);
    // TODO: Support re-alias of uncached mods (or make it unavailable).
    // TODO: Prompt to update default aliases if targets differ from defaults.
    // TODO: Confirmation dialog with list of changes.
    // TODO: Warn if about to delete uncached mod versions.

    emit textOutput("Syncing mods...");
    for (const InstalledMod &sm : toRemoveMods)
        if (!removeMod(sm))
            return false;
    for (const SpecMod &sm : toAddMods)
        if (!installMod(sm))
            return false;
    for (const SpecMod &sm : toUpdateMods)
        if (!updateMod(sm))
            return false;
    return true;
}

bool ApplyPreviewCommand::removeMod(const InstalledMod &im)
{
    QString errorInfo;
    if (app.modList().removeMod(im.id(), &errorInfo))
    {
        emit textOutput(QStringLiteral("  Removed %1 \t%2").arg(
                    util::displayInfo(im.info(), im.alias()),
                    util::displayVersion(im.info().version())));
        return true;
    }
    else
    {
        emit textOutput(QStringLiteral("Failed to remove %1: %2").arg(
                    util::displayInfo(im.info(), im.alias()), errorInfo));
        return false;
    }
}

bool ApplyPreviewCommand::installMod(const SpecMod &sm)
{
    QString errorInfo;
    const InstalledMod *im = app.modList().installMod(sm, &errorInfo);
    if (im)
    {
        emit textOutput(QStringLiteral("  Installed %1 \t%2").arg(
                    util::displayInfo(im->info(), im->alias()),
                    util::displayVersion(im->info().version())));
    }
    else
    {
        const CachedMod *cm = app.cache().mod(sm.id());
        emit textOutput(QStringLiteral("Failed to install %1: %2").arg(
                    cm ? util::displayInfo(cm->info(), sm.alias()) : util::displayInfo(sm), errorInfo));
    }
    return im;
}

bool ApplyPreviewCommand::updateMod(const SpecMod &sm)
{
    const InstalledMod *from = app.modList().mod(sm.id());
    QString fromVersion = from ? from->info().version() : QString();

    QString errorInfo;
    const InstalledMod *im = app.modList().installMod(sm, &errorInfo);
    if (im)
    {
        emit textOutput(QStringLiteral("  Installed %1 \t%2 => %3").arg(
                    util::displayInfo(im->info(), im->alias()),
                    util::displayVersion(fromVersion),
                    util::displayVersion(im->info().version())));
    }
    else
    {
        const CachedMod *cm = app.cache().mod(sm.id());
        emit textOutput(QStringLiteral("Failed to install %1: %2").arg(
                    cm ? util::displayInfo(cm->info(), sm.alias()) : util::displayInfo(sm), errorInfo));
    }
    return im;
}

} // namespace iimodmanager
