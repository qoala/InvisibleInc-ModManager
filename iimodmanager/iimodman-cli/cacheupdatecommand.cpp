#include "cacheupdatecommand.h"
#include "updatemodsimpl.h"

namespace iimodmanager {

CacheUpdateCommand::CacheUpdateCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheUpdateCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("update", "Command: Update downloaded mods currently in the cache");
    parser.addOptions({
                    {{"m", "mod-id"}, "Update mods by ID (e.g. 'workshop-2151835746'), may be repeated.", "id"},
                    {"all", "Update all mods registered in the cache."},
                    {{"a","add"}, "Automatically add missing mods to the cache."},
                    {{"f","force"}, "Overwrite even if the latest version is already downloaded."},
                    {{"y","yes"}, "Automatic yes to all prompts."},
                });
}

QFuture<void> CacheUpdateCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);
    QStringList modIds;
    impl = new UpdateModsImpl(app_, this);

    impl->setMissingCacheAction(parser.isSet("add") ? CACHE_ADD : CACHE_SKIP);
    impl->setAlreadyLatestVersionBehavior(parser.isSet("force") ? LATEST_FORCE : LATEST_SKIP);
    impl->setConfirmBeforeDownloading(!parser.isSet("yes"));

    if (parser.isSet("mod-id"))
    {
        modIds.append(parser.values("mod-id"));
    }
    if(parser.isSet("all"))
    {
        const QList<CachedMod> &mods = impl->cache().mods();
        modIds.reserve(modIds.size() + mods.size());
        for (auto mod : impl->cache().mods())
        {
            modIds.append(mod.id());
        }
    }
    modIds.removeDuplicates();


    impl->start(modIds);

    return QtFuture::connect(impl, &UpdateModsImpl::finished);
}

} // namespace iimodmanager
