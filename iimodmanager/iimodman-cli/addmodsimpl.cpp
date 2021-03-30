#include "addmodsimpl.h"

#include <QTimer>
#include <modinfo.h>

namespace iimodmanager {

AddModsImpl::AddModsImpl(ModManCliApplication &app, QObject *parent)
    : AddModsImpl(app, nullptr, nullptr, parent)
{}

AddModsImpl::AddModsImpl(ModManCliApplication &app, ModCache *cache, ModDownloader *downloader, QObject *parent)
    : QObject(parent), app(app),
      cache_(cache ? cache : new ModCache(app.config(), this)),
      downloader(downloader ? downloader : new ModDownloader(app.config(), this))
{
    if (!cache)
        cache_->refresh(ModCache::LATEST_ONLY);
}

void AddModsImpl::start(const QStringList &modIds)
{
    workshopIds = checkModIds(modIds);
    if (workshopIds.empty())
    {
        QTextStream cerr(stderr);
        cerr << QString("No mods to add to cache.") << Qt::endl;

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

QStringList AddModsImpl::checkModIds(const QStringList &modIds)
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

QString AddModsImpl::checkModId(const QString &modId)
{
    if (cache().contains(modId))
    {
        QTextStream cerr(stderr);
        cerr << QString("Mod %1 is already in the cache. Skipping.").arg(modId) << Qt::endl;
        return QString();
    }

    const ModInfo cachedInfo = ModInfo(modId);
    if (!cachedInfo.isSteam())
    {
        QTextStream cerr(stderr);
        cerr << QString("Mod %1 is not a steam workshop ID. Skipping.").arg(modId) << Qt::endl;
        return QString();
    }
    return cachedInfo.steamId();
}

void AddModsImpl::startInfos()
{
    steamInfoCall = downloader->modInfoCall();
    connect(steamInfoCall, &ModInfoCall::finished, this, &AddModsImpl::steamInfoFinished);

    loopIndex = 0;
    steamInfoCall->start(workshopIds.first());
}

void AddModsImpl::steamInfoFinished()
{
    const SteamModInfo steamInfo = steamInfoCall->result();

    const CachedMod *cachedMod = cache_->addUnloaded(steamInfo);
    QTextStream cerr(stderr);
    if (cachedMod)
        cerr << cachedMod->info().toString() << " registered to cache" << Qt::endl;
    else
        cerr << steamInfo.modId() << " couldn't be added to cache. Skipping." << Qt::endl;

    if (++loopIndex < workshopIds.size())
    {
        // Next steamInfo
        steamInfoCall->start(workshopIds.at(loopIndex));
    }
    else
    {
        // No downloads needed.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;
        emit finished();
    }
}

} // namespace iimodmanager
