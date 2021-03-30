#include "addmodsimpl.h"
#include "cacheaddcommand.h"

#include <QCommandLineParser>
#include <QFuture>

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

void CacheAddCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);

    if (parser.isSet("mod-id"))
        modIds.append(parser.values("mod-id"));
}

QFuture<void> CacheAddCommand::execute()
{
    impl = new AddModsImpl(app_, this);
    impl->start(modIds);

    return QtFuture::connect(impl, &AddModsImpl::finished);
}

} // namespace iimodmanager
