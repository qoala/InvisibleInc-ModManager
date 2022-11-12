#include "cacheupdatecommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <modcache.h>
#include <moddownloader.h>
#include <modinfo.h>

namespace iimodmanager {

CacheUpdateCommand::CacheUpdateCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app)
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
            workshopIds.append(cachedInfo.steamId());
    }

    if (workshopIds.empty())
    {
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

    loopIndex = 0;
    steamInfoCall->start(workshopIds.first());
}

void CacheUpdateCommand::nextSteamInfo()
{
    if (++loopIndex < workshopIds.size())
    {
        // Next steamInfo
        steamInfoCall->start(workshopIds.at(loopIndex));
    }
    else if (steamInfos.empty())
    {
        // No downloads needed.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;

        emit textOutput("All workshop mods are up to date.");
        emit finished();
        deleteLater();
    }
    else
    {
        // Begin downloading.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;

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
        steamInfos.append(steamInfo);
    }
    nextSteamInfo();
}

void CacheUpdateCommand::startDownloads()
{
    steamDownloadCall = app.modDownloader().modDownloadCall(app.cache());
    steamDownloadCall->setParent(this);
    connect(steamDownloadCall, &ModDownloadCall::finished, this, &CacheUpdateCommand::steamDownloadFinished);

    loopIndex = 0;
    steamDownloadCall->start(steamInfos.first());
}

void CacheUpdateCommand::nextDownload()
{
    if (++loopIndex < steamInfos.size())
    {
        // Next download.
        steamDownloadCall->start(steamInfos.at(loopIndex));
    }
    else
    {
        // Finished.
        steamDownloadCall->deleteLater();
        steamDownloadCall = nullptr;

        emit textOutput("All workshop mods are up to date.");

        app.cache().saveMetadata();
        emit finished();
        deleteLater();
    }
}

void CacheUpdateCommand::steamDownloadFinished()
{
    const CachedVersion *v = steamDownloadCall->resultVersion();
    if (v)
        emit textOutput(QString("  %1 updated.").arg(v->info().toString()));
    else
        emit textOutput(QString("  %1 download failed: %2").arg(steamDownloadCall->steamInfo().modId(), steamDownloadCall->errorDetail()));

    nextDownload();
}

} // namespace iimodmanager
