#include "installedsyncfilecommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <QFileDialog>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

InstalledSyncFileCommand::InstalledSyncFileCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app)
{}

void InstalledSyncFileCommand::execute()
{
    auto fileContentReady = [this](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty())
        {
            // Selection cancelled
            deleteLater();
        }
        else
        {
            handleFile(fileName, fileContent);
        }
    };
    QFileDialog::getOpenFileContent("Modspecs (*.txt)", fileContentReady);
}

void InstalledSyncFileCommand::handleFile(const QString &filename, const QByteArray &fileContent)
{
    emit started();
    app.refreshMods();

    inputSpec.reserve(app.cache().mods().size());
    inputSpec.appendFromFile(fileContent, filename);

    emit textOutput("\n--");
    bool ok = true;
    for (const auto &sm : inputSpec.mods())
    {
        std::optional<SpecMod> targetSpecMod = makeInstallTarget(sm);
        if (targetSpecMod)
            targetSpec.append(*targetSpecMod);
        else
            ok = false;
    }
    for (const auto &im : app.modList().mods())
        ok &= checkInstalledMod(im);

    if (ok)
      ok = doSync();

    if (!ok)
      emit textOutput("Sync aborted");

    app.refreshMods();

    emit finished();
    deleteLater();
}

std::optional<SpecMod> InstalledSyncFileCommand::makeInstallTarget(const SpecMod &sm)
{
    const InstalledMod *im = app.modList().mod(sm.id());
    const CachedMod *cm = app.cache().mod(sm.id());
    if (!cm && im && sm.versionId().isEmpty())
    {
        // Assume we're just using the existing version.
        return im->asSpec();
    }
    else if (!cm)
    {
        emit textOutput(QStringLiteral("  Mod does not exist in cache: %1 [%2]").arg(sm.name()).arg(sm.id()));
        return {};
    }
    if (!cm->downloaded())
    {
        emit textOutput(QStringLiteral("  Mod has no downloaded versions: %1").arg(cm->info().toString()));
        return {};
    }

    const CachedVersion *cv = sm.versionId().isEmpty() ? cm->latestVersion() : app.cache().refreshVersion(sm.id(), sm.versionId());
    if (!cv)
    {
        emit textOutput(QStringLiteral("  Mod version is not in cache: %1 %2").arg(cm->info().toString()).arg(sm.versionId()));
        return {};
    }

    SpecMod targetMod = cv->asSpec();

    if (!im)
        addedMods.append(targetMod);
    else if (!cv->installed())
        updatedMods.append(targetMod);

    return targetMod;
}

bool InstalledSyncFileCommand::checkInstalledMod(const InstalledMod &im)
{
    const QString &modId = im.id();
    if (targetSpec.contains(modId))
        return true;

    removedMods.append(im);

    // if (!im.hasCacheVersion()) warning

    return true;
}

bool InstalledSyncFileCommand::doSync()
{
    emit textOutput("Syncing mods...");
    for (const InstalledMod &sm : removedMods)
        if (!removeMod(sm))
            return false;
    for (const SpecMod &sm : addedMods)
        if (!installMod(sm))
            return false;
    for (const SpecMod &sm : updatedMods)
        if (!updateMod(sm))
            return false;
    emit textOutput("Sync complete");
    return true;
}

bool InstalledSyncFileCommand::removeMod(const InstalledMod &im)
{
    QString errorInfo;
    if (app.modList().removeMod(im.id(), &errorInfo))
    {
        emit textOutput(QStringLiteral("  %1 removed \t%2").arg(
                    im.info().toString(),
                    util::displayVersion(im.info().version())));
        return true;
    }
    else
    {
        emit textOutput(QStringLiteral("Failed to remove %1: %2").arg(
                    im.info().toString(), errorInfo));
        return false;
    }
}

bool InstalledSyncFileCommand::installMod(const SpecMod &sm)
{
    QString errorInfo;
    const InstalledMod *im = app.modList().installMod(sm, &errorInfo);
    if (im)
    {
        emit textOutput(QStringLiteral("  %1 installed \t%2").arg(
                    im->info().toString(),
                    util::displayVersion(im->info().version())));
    }
    else
    {
        const CachedMod *cm = app.cache().mod(sm.id());
        emit textOutput(QStringLiteral("Failed to install %1: %2").arg(
                    cm ? cm->info().toString() : sm.id(), errorInfo));
    }
    return im;
}

bool InstalledSyncFileCommand::updateMod(const SpecMod &sm)
{
    const InstalledMod *from = app.modList().mod(sm.id());
    QString fromVersion = from ? from->info().version() : QString();

    QString errorInfo;
    const InstalledMod *im = app.modList().installMod(sm, &errorInfo);
    if (im)
    {
        emit textOutput(QStringLiteral("  %1 installed \t%2 => %3").arg(
                    im->info().toString(),
                    util::displayVersion(fromVersion),
                    util::displayVersion(im->info().version())));
    }
    else
    {
        const CachedMod *cm = app.cache().mod(sm.id());
        emit textOutput(QStringLiteral("Failed to install %1: %2").arg(
                    cm ? cm->info().toString() : sm.id(), errorInfo));
    }
    return im;
}

} // namespace iimodmanager

