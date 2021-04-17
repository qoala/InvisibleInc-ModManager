#include "cacheaddinstalledcommand.h"
#include "updatemodsimpl.h"

#include <QCommandLineParser>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

CacheAddInstalledCommand::CacheAddInstalledCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheAddInstalledCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("add-installed", "Command: Add all currently installed mods to the cache.");
}

void CacheAddInstalledCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(parser);
    Q_UNUSED(args);
}

void CacheAddInstalledCommand::execute()
{
    cache = new ModCache(app_.config(), this);
    modList = new ModList(app_.config(), cache, this);
    updateImpl = new UpdateModsImpl(app_, cache, nullptr, this);
    updateImpl->setVerb(UpdateModsImpl::VERB_DOWNLOAD);
    updateImpl->setMissingCacheAction(UpdateModsImpl::CACHE_ADD);

    cache->refresh(ModCache::LATEST_ONLY);
    modList->refresh();

    // all mods to be added
    modIds.clear();
    modIds.reserve(modList->mods().size());
    // mods that will be found on steam
    QStringList steamModIds;
    steamModIds.reserve(modList->mods().size());
    for (const InstalledMod &m : modList->mods())
    {
        modIds.append(m.id());
        if (m.info().isSteam())
            steamModIds.append(m.id());
    }

    updateImpl->start(steamModIds);
    connect(updateImpl, &UpdateModsImpl::finished, this, &CacheAddInstalledCommand::updateFinished);
}

void CacheAddInstalledCommand::updateFinished()
{
    if (!updateImpl->success())
    {
        // Abort the operation.
        emit finished();
        return;
    }

    emit finished();
}

} // namespace iimodmanager
