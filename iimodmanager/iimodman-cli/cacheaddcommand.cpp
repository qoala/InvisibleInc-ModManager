#include "addmodsimpl.h"
#include "cacheaddcommand.h"

namespace iimodmanager {

CacheAddCommand::CacheAddCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheAddCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("update", "Command: Update downloaded mods currently in the cache");
    parser.addOptions({
                    {{"m", "mod-id"}, "Add mods by ID (e.g. 'workshop-2151835746'), may be repeated.", "id"},
                });
}

QFuture<void> CacheAddCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);
    QStringList modIds;
    impl = new AddModsImpl(app_, this);

    if (parser.isSet("mod-id"))
    {
        modIds.append(parser.values("mod-id"));
    }

    impl->start(modIds);

    return QtFuture::connect(impl, &AddModsImpl::finished);
}

} // namespace iimodmanager
