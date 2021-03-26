#include "updatemodsimpl.h"

#include <QDir>

namespace iimodmanager {

UpdateModsImpl::UpdateModsImpl(ModManCliApplication &app, QObject *parent)
    : QObject(parent), app(app), cache_(app.config()), downloader(app.config())
{
    cache_.refresh();
}

void UpdateModsImpl::start(const QStringList &modIds)
{
    steamInfoCall = downloader.modInfoCall();
    steamDownloadCall = downloader.modDownloadCall();

    const QString &modId = modIds.first();
    if (!cache_.contains(modId))
    {
        QTextStream cerr(stderr);
        cerr << QString("Mod %1 is not in the cache. Run 'cache add --id %1' first.").arg(modId) << Qt::endl;
        exit(EXIT_FAILURE);
    }
    const CachedMod *cachedMod = &cache_.mod(modId);
    const Mod &cachedInfo = cachedMod->info();
    if (!cachedInfo.isSteam())
    {
        QTextStream cerr(stderr);
        cerr << QString("Mod %1 is not from Steam.").arg(cachedInfo.toString()) << Qt::endl;
        exit(EXIT_FAILURE);
    }
    const QString workshopId = cachedInfo.steamId();

    steamInfoCall->start(workshopId);

    connect(steamInfoCall, &ModInfoCall::finished, this, [this, cachedMod]
    {
        const SteamModInfo steamInfo = steamInfoCall->result();
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;

        if (alreadyLatestVersionBehavior == SKIP && cachedMod->containsVersion(steamInfo.lastUpdated))
        {
            QTextStream cerr(stderr);
            cerr << cachedMod->info().toString() << " already up to date" << Qt::endl;

            steamDownloadCall->deleteLater();
            steamDownloadCall = nullptr;
            emit finished();
        }
        else
        {
            steamDownloadCall->start(steamInfo);
        }
    });

    connect(steamDownloadCall, &ModDownloadCall::finished, this, [this]
    {
        QDir modVersionDir(steamDownloadCall->resultPath());

        QTextStream cerr(stderr);
        QFile modInfoFile = QFile(modVersionDir.absoluteFilePath("modinfo.txt"));
        Mod mod = Mod::readModInfo(modInfoFile, steamDownloadCall->modInfo().modId());
        cerr << mod.toString() << " updated" << Qt::endl;

        steamDownloadCall->deleteLater();
        steamDownloadCall = nullptr;
        emit finished();
    });
}

} // namespace iimodmanager
