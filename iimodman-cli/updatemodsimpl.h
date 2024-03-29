#ifndef IIMODMANAGER_UPDATEMODSIMPL_H
#define IIMODMANAGER_UPDATEMODSIMPL_H

#include "modmancliapplication.h"


namespace iimodmanager {

class ConfirmationPrompt;
class ModCache;
class ModDownloadCall;
class ModDownloader;
class ModInfoCall;
struct SteamModInfo;

//! Shared implementation for commands that update mods in the mod cache.
class UpdateModsImpl : public QObject
{
    Q_OBJECT

public:
    enum ActionVerb {
        VERB_UPDATE,
        VERB_DOWNLOAD,
    };

    enum MissingCacheAction {
        CACHE_SKIP,
        CACHE_ADD,
    };

    enum AlreadyLatestVersionAction {
        LATEST_SKIP,
        LATEST_FORCE
    };

    UpdateModsImpl(ModManCliApplication &app, QObject *parent = nullptr);
    UpdateModsImpl(ModManCliApplication &app, ModCache *cache, ModDownloader *downloader, QObject *parent = nullptr);

    const ModCache &cache() const { return *cache_; };
    bool success() const { return success_; };

    void setVerb(ActionVerb);
    void setMissingCacheAction(MissingCacheAction);
    void setAlreadyLatestVersionBehavior(AlreadyLatestVersionAction);
    void setConfirmBeforeDownloading(bool);
    void setVerbose(bool);

    void start(const QStringList &modIds);

signals:
    void finished();

private:
    ModManCliApplication &app;
    ModCache *cache_;
    ModDownloader *downloader;
    ActionVerb verb;
    MissingCacheAction missingCacheAction;
    AlreadyLatestVersionAction alreadyLatestVersionAction;
    bool confirmBeforeDownloading;
    bool verbose;

    bool success_;
    ModInfoCall *steamInfoCall;
    ModDownloadCall *steamDownloadCall;
    ConfirmationPrompt *prompt;
    QStringList workshopIds;
    QList<SteamModInfo> steamInfos;
    qsizetype loopIndex;

    QStringList checkModIds(const QStringList &modIds);
    QString checkModId(const QString &modId);
    void startInfos();
    void nextSteamInfo();
    void confirmDownloads();
    void startDownloads();
    void nextDownload();

    void steamInfoFinished();
    void steamDownloadFinished();
};

inline void UpdateModsImpl::setVerb(UpdateModsImpl::ActionVerb verb)
{
    this->verb = verb;
}

inline void UpdateModsImpl::setMissingCacheAction(MissingCacheAction action)
{
    missingCacheAction = action;
}

inline void UpdateModsImpl::setAlreadyLatestVersionBehavior(AlreadyLatestVersionAction action)
{
    alreadyLatestVersionAction = action;
}

inline void UpdateModsImpl::setConfirmBeforeDownloading(bool behavior)
{
    confirmBeforeDownloading = behavior;
}

inline void UpdateModsImpl::setVerbose(bool behavior)
{
    verbose = behavior;
}

} // namespace iimodmanager

#endif // IIMODMANAGER_UPDATEMODSIMPL_H
