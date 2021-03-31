#include "updatemodsimpl.h"

#include <iostream>
#include <QDir>
#include <QTimer>
#include <QThreadPool>
#include <modinfo.h>

namespace iimodmanager {

UpdateModsImpl::UpdateModsImpl(ModManCliApplication &app, QObject *parent)
    : UpdateModsImpl(app, nullptr, nullptr, parent)
{}

UpdateModsImpl::UpdateModsImpl(ModManCliApplication &app, ModCache *cache, ModDownloader *downloader, QObject *parent)
    : QObject(parent), app(app),
      cache_(cache ? cache : new ModCache(app.config(), this)),
      downloader(downloader ? downloader : new ModDownloader(app.config(), this)),
      missingCacheAction(CACHE_SKIP),
      alreadyLatestVersionAction(LATEST_SKIP),
      confirmBeforeDownloading(true)
{}

void UpdateModsImpl::start(const QStringList &modIds)
{
    if (cache_->mods().isEmpty())
        cache_->refresh(ModCache::LATEST_ONLY);

    workshopIds = checkModIds(modIds);
    if (workshopIds.empty())
    {
        QTextStream cerr(stderr);
        cerr << QString("No mods to update.") << Qt::endl;

        // Use a single-shot timer in case the event loop hasn't started yet.
        QTimer::singleShot(0, this, [this]{
            emit finished();
        });
    }
    else
    {
        startInfos();
    }
}

QStringList UpdateModsImpl::checkModIds(const QStringList &modIds)
{
    QStringList workshopIds;
    workshopIds.reserve(modIds.size());
    for (auto modId : modIds)
    {
        QString workshopId = checkModId(modId);
        if (!workshopId.isEmpty())
        {
            workshopIds.append(workshopId);
        }
    }
    return workshopIds;
}

QString UpdateModsImpl::checkModId(const QString &modId)
{
    ModInfo cachedInfo;
    if (const CachedMod *cachedMod = cache().mod(modId))
    {
        cachedInfo = cachedMod->info();
    }
    else if (missingCacheAction == CACHE_ADD)
    {
        cachedInfo = ModInfo(modId);
    }
    else
    {
        QTextStream cerr(stderr);
        cerr << QString("Mod %1 is not in the cache. Skipping.").arg(modId) << Qt::endl;
        return QString();
    }

    if (!cachedInfo.isSteam())
    {
        QTextStream cerr(stderr);
        cerr << QString("Mod %1 is not a steam workshop ID. Skipping.").arg(modId) << Qt::endl;
        return QString();
    }
    return cachedInfo.steamId();
}

void UpdateModsImpl::startInfos()
{
    steamInfoCall = downloader->modInfoCall();
    connect(steamInfoCall, &ModInfoCall::finished, this, &UpdateModsImpl::steamInfoFinished);

    steamInfos.clear();
    steamInfos.reserve(workshopIds.size());

    loopIndex = 0;
    steamInfoCall->start(workshopIds.first());
}

void UpdateModsImpl::nextSteamInfo()
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
        emit finished();
    }
    else
    {
        // Begin downloading.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;
        if (confirmBeforeDownloading)
            confirmDownloads();
        else
            startDownloads();
    }
}

void UpdateModsImpl::confirmDownloads()
{
    // Run on another thread, as std::getline blocks.
    QThreadPool::globalInstance()->start([this]{
        QTextStream cerr(stderr);
        cerr << "The following mods will be downloaded:" << Qt::endl << "  ";
        for (auto steamInfo : steamInfos)
        {
        cerr << QString("\"%1\" [%2] ").arg(steamInfo.title, steamInfo.modId());
        }
        cerr << Qt::endl;
        cerr << "Do you want to continue? [Y/n] " << Qt::flush;

        QTextStream cin(stdin);
        QString response = cin.readLine();
        if (response.isEmpty() || response == "y" || response == "yes")
        {
            // Start downloads on the main thread.
            QTimer::singleShot(0, this, &UpdateModsImpl::startDownloads);
        }
        else
        {
            cerr << "Abort." << Qt::endl;
            emit finished();
        }
    });
}

void UpdateModsImpl::startDownloads()
{
    steamDownloadCall = downloader->modDownloadCall();
    connect(steamDownloadCall, &ModDownloadCall::finished, this, &UpdateModsImpl::steamDownloadFinished);

    loopIndex = 0;
    steamDownloadCall->start(steamInfos.first());
}

void UpdateModsImpl::nextDownload()
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
        emit finished();
    }
}

void UpdateModsImpl::steamInfoFinished()
{
    const SteamModInfo steamInfo = steamInfoCall->result();

    if (!steamInfo.valid())
    {
        QTextStream cerr(stderr);
        cerr << "workshop-" << steamInfoCall->workshopId() << " couldn't be added to cache. Skipping." << Qt::endl;
        nextSteamInfo();
        return;
    }

    const QString modId = steamInfo.modId();
    const CachedMod *cachedMod = cache().contains(modId) ? cache().mod(modId) : cache_->addUnloaded(steamInfo);
    if (!cachedMod)
    {
        QTextStream cerr(stderr);
        cerr << modId << " couldn't be added to cache. Skipping." << Qt::endl;
    }
    else if (alreadyLatestVersionAction == LATEST_SKIP && cachedMod->containsVersion(steamInfo.lastUpdated))
    {
        QTextStream cerr(stderr);
        cerr << cachedMod->info().toString() << " already up to date" << Qt::endl;
    }
    else
    {
        steamInfos.append(steamInfo);
    }

    nextSteamInfo();
}

void UpdateModsImpl::steamDownloadFinished()
{
    QDir modVersionDir(steamDownloadCall->resultPath());

    QTextStream cerr(stderr);
    QFile modInfoFile = QFile(modVersionDir.absoluteFilePath("modinfo.txt"));
    ModInfo mod = ModInfo::readModInfo(modInfoFile, steamDownloadCall->modInfo().modId());
    cerr << mod.toString() << " updated" << Qt::endl;

    nextDownload();
}

} // namespace iimodmanager
