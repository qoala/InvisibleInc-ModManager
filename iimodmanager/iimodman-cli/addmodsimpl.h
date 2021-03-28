#ifndef IIMODMANAGER_ADDMODSIMPL_H
#define IIMODMANAGER_ADDMODSIMPL_H

#include "modmancliapplication.h"

#include <modcache.h>
#include <moddownloader.h>


namespace iimodmanager {

//! Shared implementation for commands that register mods in the mod cache from Steam.
class AddModsImpl : public QObject
{
    Q_OBJECT
public:
    AddModsImpl(ModManCliApplication &app, QObject *parent = nullptr);
    AddModsImpl(ModManCliApplication &app, ModCache *cache, ModDownloader *downloader, QObject *parent = nullptr);

    const ModCache &cache() const { return *cache_; };

    void start(const QStringList &modIds);

signals:
    void finished();

private:
    ModManCliApplication &app;
    ModCache *cache_;
    ModDownloader *downloader;

    ModInfoCall *steamInfoCall;
    QStringList workshopIds;
    qsizetype loopIndex;

    QStringList checkModIds(const QStringList &modIds);
    QString checkModId(const QString &modId);
    void startInfos();

    void steamInfoFinished();
};

} // namespace iimodmanager

#endif // IIMODMANAGER_ADDMODSIMPL_H
