#include "addmodsimpl.h"

#include <QTimer>

namespace iimodmanager {

AddModsImpl::AddModsImpl(ModManCliApplication &app, QObject *parent)
    : QObject(parent), app(app), cache_(app.config()), downloader(app.config())
{
    cache_.refresh();
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
    if (cache_.contains(modId))
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
    steamInfoCall = downloader.modInfoCall();
    connect(steamInfoCall, &ModInfoCall::finished, this, &AddModsImpl::steamInfoFinished);

    loopIndex = 0;
    steamInfoCall->start(workshopIds.first());
}

void AddModsImpl::steamInfoFinished()
{
    const SteamModInfo steamInfo = steamInfoCall->result();

    const QString modId = steamInfo.modId();
    const CachedMod &cachedMod = cache_.addUnloaded(steamInfo);
    QTextStream cerr(stderr);
    cerr << cachedMod.info().toString() << " registered to cache" << Qt::endl;

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