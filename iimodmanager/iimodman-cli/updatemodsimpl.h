#ifndef IIMODMANAGER_UPDATEMODSIMPL_H
#define IIMODMANAGER_UPDATEMODSIMPL_H

#include "modmancliapplication.h"

#include <QPromise>
#include <modcache.h>
#include <moddownloader.h>



namespace iimodmanager {

enum AlreadyLatestVersionBehavior {
    SKIP,
    FORCE_UPDATE
};

//! Shared implementation for commands that update the mod cache.
class UpdateModsImpl : public QObject
{
    Q_OBJECT

public:
    UpdateModsImpl(ModManCliApplication &app, QObject *parent = nullptr);

    const ModCache &cache() const { return cache_; };

    void setAlreadyLatestVersionBehavior(AlreadyLatestVersionBehavior behavior);

    void start(const QStringList &modIds);

signals:
    void finished();

private:
    ModManCliApplication &app;
    ModCache cache_;
    ModDownloader downloader;
    AlreadyLatestVersionBehavior alreadyLatestVersionBehavior;

    ModInfoCall *steamInfoCall;
    ModDownloadCall *steamDownloadCall;
    QStringList workshopIds;
    QList<SteamModInfo> steamInfos;
    qsizetype loopIndex;

    QStringList checkModIds(const QStringList &modIds);
    QString checkModId(const QString &modId);

    void steamInfoFinished();
    void steamDownloadFinished();
};

inline void UpdateModsImpl::setAlreadyLatestVersionBehavior(AlreadyLatestVersionBehavior behavior)
{
    alreadyLatestVersionBehavior = behavior;
}

} // namespace iimodmanager

#endif // IIMODMANAGER_UPDATEMODSIMPL_H
