#include "cacheupdatecommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <moddownloader.h>
#include <modinfo.h>

namespace iimodmanager {

CacheUpdateCommand::CacheUpdateCommand(ModManGuiApplication  &app, QTextCursor cursor, QObject *parent)
  : QObject(parent), app(app), cursor(cursor)
{}

void CacheUpdateCommand::execute()
{
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n---\n");

    app.cache().refresh(ModCache::LATEST_ONLY);
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
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("No workshop mods in the cache. Try adding new mods first.\n\n");
        emit finished();
        deleteLater();
    }
    else
    {
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("Checking for updates...\n");
        startInfos();
    }
}

void CacheUpdateCommand::startInfos()
{
    steamInfoCall = app.modDownloader().modInfoCall();
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

        cursor.movePosition(QTextCursor::End);
        cursor.insertText(QString("All workshop mods are up to date.\n"));
        emit finished();
        deleteLater();
    }
    else
    {
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(QString("Found %1 updates\n").arg(steamInfos.size()));
        emit finished();
        deleteLater();
    }
}

void CacheUpdateCommand::steamInfoFinished()
{
    const SteamModInfo &steamInfo = steamInfoCall->result();
    if (!steamInfo.valid())
    {
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(QString("  workshop-%1 couldn't be retrieved. Skipping.\n").arg(steamInfoCall->workshopId()));
        nextSteamInfo();
        return;
    }

    const QString modId = steamInfo.modId();
    const CachedMod *cachedMod = app.cache().mod(modId);
    if (!cachedMod)
    {
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(QString("  workshop-%1 cache error. Skipping.\n"));
    }
    else if (!cachedMod->containsVersion(steamInfo.lastUpdated))
    {
        steamInfos.append(steamInfo);
    }
    nextSteamInfo();
}

} // namespace iimodmanager
