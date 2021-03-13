#include "cachelistcommand.h"

#include <QTextStream>
#include <mod.h>
#include <modlist.h>

namespace iimodmanager {

CacheListCommand::CacheListCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("list", "Command: List all downloaded mods in the cache");
}

void CacheListCommand::execute(QCommandLineParser &parser, const QStringList &args) const
{
    Q_UNUSED(parser);
    Q_UNUSED(args);

    ModList mods = ModList::readCurrent(app_.config(), CACHED);

    QTextStream cout(stdout);
    for (auto const& mod : mods.list()) {
        cout << mod.id() << ":" << mod.name() << Qt::endl;
    }
}

}  // namespace iimodmanager
