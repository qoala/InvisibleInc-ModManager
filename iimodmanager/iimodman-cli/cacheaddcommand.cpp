#include "addmodsimpl.h"
#include "cacheaddcommand.h"

#include <QCommandLineParser>

namespace iimodmanager {

CacheAddCommand::CacheAddCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheAddCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("add", "Command: Register mods to the cache for later download");
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

void CacheAddCommand::execute()
{
    impl = new AddModsImpl(app_, this);
    impl->start(modIds);

    connect(impl, &AddModsImpl::finished, this, &Command::finished);
}

} // namespace iimodmanager
