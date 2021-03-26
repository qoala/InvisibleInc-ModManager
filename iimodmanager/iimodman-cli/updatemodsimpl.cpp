#include "updatemodsimpl.h"

#include <QDir>
#include <QTimer>

namespace iimodmanager {

UpdateModsImpl::UpdateModsImpl(ModManCliApplication &app, QObject *parent)
    : QObject(parent), app(app), cache_(app.config()), downloader(app.config())
{
    cache_.refresh();
}

void UpdateModsImpl::start(const QStringList &modIds)
{
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

    steamInfos.clear();
    steamInfos.reserve(workshopIds.size());
    steamInfoCall = downloader.modInfoCall();
    steamDownloadCall = downloader.modDownloadCall();

    loopIndex = 0;
    steamInfoCall->start(workshopIds.first());

    connect(steamInfoCall, &ModInfoCall::finished, this, &UpdateModsImpl::steamInfoFinished);
    connect(steamDownloadCall, &ModDownloadCall::finished, this, &UpdateModsImpl::steamDownloadFinished);
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
        if (!cache_.contains(modId))
        {
            QTextStream cerr(stderr);
            cerr << QString("Mod %1 is not in the cache. Run 'cache add --id %1' first.").arg(modId) << Qt::endl;
            return QString();
        }
        const CachedMod &cachedMod = cache_.mod(modId);
        const Mod &cachedInfo = cachedMod.info();
        if (!cachedInfo.isSteam())
        {
            QTextStream cerr(stderr);
            cerr << QString("Mod %1 is not from Steam.").arg(cachedInfo.toString()) << Qt::endl;
            return QString();
        }
        return cachedInfo.steamId();
}

void UpdateModsImpl::steamInfoFinished()
{
    const SteamModInfo steamInfo = steamInfoCall->result();

    const CachedMod &cachedMod = cache().mod(steamInfo.modId());
    if (alreadyLatestVersionBehavior == SKIP && cachedMod.containsVersion(steamInfo.lastUpdated))
    {
        QTextStream cerr(stderr);
        cerr << cachedMod.info().toString() << " already up to date" << Qt::endl;
    }
    else
    {
        steamInfos.append(steamInfo);
    }

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
        steamDownloadCall->deleteLater();
        steamDownloadCall = nullptr;
        emit finished();
    }
    else
    {
        // Begin downloading.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;
        loopIndex = 0;
        steamDownloadCall->start(steamInfos.first());
    }
}

void UpdateModsImpl::steamDownloadFinished()
{
    QDir modVersionDir(steamDownloadCall->resultPath());

    QTextStream cerr(stderr);
    QFile modInfoFile = QFile(modVersionDir.absoluteFilePath("modinfo.txt"));
    Mod mod = Mod::readModInfo(modInfoFile, steamDownloadCall->modInfo().modId());
    cerr << mod.toString() << " updated" << Qt::endl;

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

} // namespace iimodmanager
