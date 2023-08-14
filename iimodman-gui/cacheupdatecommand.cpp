#include "cacheupdatecommand.h"
#include "guidownloader.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <modcache.h>
#include <moddownloader.h>
#include <modinfo.h>

namespace iimodmanager {

CacheUpdateCommand::CacheUpdateCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app), onlyAlreadyPending(false)
{}

CacheUpdateCommand::CacheUpdateCommand(ModManGuiApplication  &app, bool onlyAlreadyPending, QObject *parent)
  : QObject(parent), app(app), onlyAlreadyPending(onlyAlreadyPending)
{}

void CacheUpdateCommand::execute()
{
    app.refreshMods();
    const QList<CachedMod> &mods = app.cache().mods();
    workshopIds.clear();
    workshopIds.reserve(mods.size());
    for (auto mod : mods)
    {
        const ModInfo &cachedInfo = mod.info();
        if (cachedInfo.isSteam())
        {
            if (!onlyAlreadyPending || mod.availableVersion())
                workshopIds.append(cachedInfo.steamId());
        }
    }

    if (workshopIds.empty())
    {
        if (onlyAlreadyPending)
            emit textOutput("No workshop mods pending update. Try checking for updates first.");
        else
            emit textOutput("No workshop mods in the cache. Try adding new mods first.");
        emit finished();
        deleteLater();
    }
    else
    {
        emit textOutput("Checking for updates...");
        startInfos();
    }
}

void CacheUpdateCommand::startInfos()
{
    steamInfoCall = app.modDownloader().modInfoCall();
    steamInfoCall->setParent(this);
    connect(steamInfoCall, &ModInfoCall::finished, this, &CacheUpdateCommand::steamInfoFinished);

    steamInfos.clear();
    steamInfos.reserve(workshopIds.size());
    emit beginProgress(workshopIds.size());

    loopIndex = 0;
    steamInfoCall->start(workshopIds.first());
}

void CacheUpdateCommand::nextSteamInfo()
{
    if (++loopIndex < workshopIds.size())
    {
        // Next steamInfo
        emit updateProgress(loopIndex);
        steamInfoCall->start(workshopIds.at(loopIndex));
    }
    else if (steamInfos.empty())
    {
        // No downloads needed.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;

        emit updateProgress(loopIndex);
        emit textOutput("All workshop mods are up to date.");
        emit finished();
        deleteLater();
    }
    else
    {
        // Begin downloading.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;

        // Save the existence of available versions early.
        app.cache().saveMetadata();

        emit beginProgress(steamInfos.size());
        emit textOutput(QString("Found %1 updates. Downloading...").arg(steamInfos.size()));
        startDownloads();
    }
}

void CacheUpdateCommand::steamInfoFinished()
{
    const SteamModInfo &steamInfo = steamInfoCall->result();
    if (!steamInfo.valid())
    {
        emit textOutput(QString("  Skipping workshop-%1: %2").arg(steamInfoCall->workshopId(), steamInfoCall->errorDetail()));
        nextSteamInfo();
        return;
    }

    const QString modId = steamInfo.modId();
    const CachedMod *cachedMod = app.cache().mod(modId);
    if (!cachedMod)
    {
        emit textOutput(QString("  Skipping %1: cache error.").arg(modId));
    }
    else if (!cachedMod->containsVersion(steamInfo.lastUpdated))
    {
        app.cache().markAvailableVersion(modId, steamInfo.lastUpdated);
        steamInfos.append(steamInfo);
    }
    nextSteamInfo();
}

void CacheUpdateCommand::startDownloads()
{
    auto *downloader = new GuiModDownloader(app, steamInfos, this);
    connect(downloader, &GuiModDownloader::finished, this, &CacheUpdateCommand::modDownloadFinished);
    connect(downloader, &GuiModDownloader::textOutput, this, &CacheUpdateCommand::textOutput);
    connect(downloader, &GuiModDownloader::beginProgress, this, &CacheUpdateCommand::beginProgress);
    connect(downloader, &GuiModDownloader::updateProgress, this, &CacheUpdateCommand::updateProgress);

    downloader->execute();
}

void CacheUpdateCommand::modDownloadFinished()
{
    emit textOutput("All workshop mods are up to date.");

    app.cache().saveMetadata();
    emit finished();
    deleteLater();
}

} // namespace iimodmanager
