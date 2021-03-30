#include "cacheupdatecommand.h"
#include "modmancliapplication.h"
#include "updatemodsimpl.h"

#include <QCommandLineParser>
#include <modcache.h>

namespace iimodmanager {

CacheUpdateCommand::CacheUpdateCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheUpdateCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("update", "Command: Update downloaded mods currently in the cache");
    parser.addOptions({
                          {{"m", "mod-id"}, "Update mods by ID (e.g. 'workshop-2151835746'),\nmay be repeated.", "id"},
                          {"all", "Update all mods registered in the cache."},
                          {"downloaded", "Update all currently downloaded mods in the cache.\n(default if no mods specified)"},
                          {{"a","add"}, "Automatically add missing mods to the cache."},
                          {{"f","force"}, "Overwrite even if the latest version is already downloaded."},
                          {{"y","yes"}, "Automatic yes to all prompts."},
                      });
}

void addDownloaded(QStringList &modIds, const ModCache &cache)
{
    const QList<CachedMod> &mods = cache.mods();
    modIds.reserve(modIds.size() + mods.size());
    for (auto mod : mods)
    {
        if (mod.downloaded())
            modIds.append(mod.id());
    }
}

void addAllCached(QStringList &modIds, const ModCache &cache)
{
    const QList<CachedMod> &mods = cache.mods();
    modIds.reserve(modIds.size() + mods.size());
    for (auto mod : mods)
        modIds.append(mod.id());
}

void CacheUpdateCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);
    cache = new ModCache(app_.config(), this);
    impl = new UpdateModsImpl(app_, cache, nullptr, this);

    impl->setMissingCacheAction(parser.isSet("add") ? CACHE_ADD : CACHE_SKIP);
    impl->setAlreadyLatestVersionBehavior(parser.isSet("force") ? LATEST_FORCE : LATEST_SKIP);
    impl->setConfirmBeforeDownloading(!parser.isSet("yes"));

    if (parser.isSet("mod-id"))
        modIds.append(parser.values("mod-id"));
    if(parser.isSet("downloaded"))
        inclDownloaded = true;
    if(parser.isSet("all"))
        inclAll = true;
}

void CacheUpdateCommand::execute()
{
    cache->refresh(ModCache::LATEST_ONLY);

    if (inclDownloaded)
        addDownloaded(modIds, impl->cache());
    if (inclAll)
        addAllCached(modIds, impl->cache());
    modIds.removeDuplicates();

    // --downloaded by default
    if (modIds.isEmpty())
        addDownloaded(modIds, impl->cache());

    impl->start(modIds);

    connect(impl, &UpdateModsImpl::finished, this, &Command::finished);
}

} // namespace iimodmanager
