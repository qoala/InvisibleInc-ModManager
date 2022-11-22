#include "guidownloader.h"
#include "modmanguiapplication.h"

#include <modcache.h>
#include <moddownloader.h>
#include <modinfo.h>

namespace iimodmanager {

GuiModDownloader::GuiModDownloader(ModManGuiApplication &app, const QList<SteamModInfo> &steamInfos, QObject *parent)
    : QObject(parent), steamInfos(steamInfos), loopIndex(0)
{
    steamDownloadCall = app.modDownloader().modDownloadCall(app.cache());
    steamDownloadCall->setParent(this);
    connect(steamDownloadCall, &ModDownloadCall::finished, this, &GuiModDownloader::steamDownloadFinished);
}

void GuiModDownloader::execute()
{
    if (loopIndex != 0)
        return;

    steamDownloadCall->start(steamInfos.first());
}

void GuiModDownloader::steamDownloadFinished()
{
    const CachedVersion *v = steamDownloadCall->resultVersion();
    if (v)
    {
        emit textOutput(QString("  %1 downloaded.").arg(v->info().toString()));
    }
    else
    {
        emit textOutput(QString("  %1 download failed: %2").arg(steamDownloadCall->steamInfo().modId(), steamDownloadCall->errorDetail()));
    }

    // Next.
    if (++loopIndex < steamInfos.size())
    {
        // Next download.
        emit updateProgress(loopIndex);
        steamDownloadCall->start(steamInfos.at(loopIndex));
    }
    else
    {
        // Finished.
        steamDownloadCall->deleteLater();
        steamDownloadCall = nullptr;

        emit updateProgress(loopIndex);
        emit finished();
        deleteLater();
    }
}

} // namespace iimodmanager
