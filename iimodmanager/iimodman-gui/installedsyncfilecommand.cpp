#include "installedsyncfilecommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <QFileDialog>
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
            emit finished();
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
    app.refreshMods();

    inputSpec.reserve(app.cache().mods().size());
    inputSpec.appendFromFile(fileContent, filename);

    emit textOutput("\n--");
    bool success = true;
    for (const auto &sm : inputSpec.mods())
    {
        std::optional<SpecMod> targetSpecMod = makeInstallTarget(sm);
        if (targetSpecMod)
            targetSpec.append(*targetSpecMod);
        else
            success = false;
    }
    for (const auto &im : app.modList().mods())
        success &= checkInstalledMod(im);

    if (success)
      doSync();
    else
      emit textOutput("Sync aborted");

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
        emit textOutput(QString("  Mod does not exist in cache: %1 [%2]").arg(sm.name()).arg(sm.id()));
        return {};
    }
    if (!cm->downloaded())
    {
        emit textOutput(QString("  Mod has no downloaded versions: %1").arg(cm->info().toString()));
        return {};
    }

    const CachedVersion *cv = sm.versionId().isEmpty() ? cm->latestVersion() : app.cache().refreshVersion(sm.id(), sm.versionId());
    if (!cv)
    {
        emit textOutput(QString("  Mod version is not in cache: %1 %2").arg(cm->info().toString()).arg(sm.versionId()));
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

void InstalledSyncFileCommand::doSync()
{
    emit textOutput("Syncing mods...");
    for (const InstalledMod &sm : removedMods)
        if (!removeMod(sm))
            return;
    for (const SpecMod &sm : addedMods)
        if (!installMod(sm))
            return;
    for (const SpecMod &sm : updatedMods)
        if (!installMod(sm))
            return;
    emit textOutput("Sync complete");
}

bool InstalledSyncFileCommand::removeMod(const InstalledMod &im)
{
    QString errorInfo;
    if (app.modList().removeMod(im.id(), &errorInfo))
    {
        emit textOutput(QString("  %1 removed").arg(im.info().toString()));
        return true;
    }
    else
    {
        emit textOutput(QString("Failed to remove %1: %2").arg(im.info().toString(), errorInfo));
        return false;
    }
}

bool InstalledSyncFileCommand::installMod(const SpecMod &sm)
{
    QString errorInfo;
    const InstalledMod *im = app.modList().installMod(sm, &errorInfo);
    if (im)
    {
        emit textOutput(QString("  %1 installed").arg(im->info().toString()));
    }
    else
    {
        const CachedMod *cm = app.cache().mod(sm.id());
        emit textOutput(QString("Failed to install %1: %2").arg(cm ? cm->info().toString() : sm.id(), errorInfo));
    }
    return im;
}

} // namespace iimodmanager

