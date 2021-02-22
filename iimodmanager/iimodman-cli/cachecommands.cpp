#include "cachecommands.h"
#include "cachelistcommand.h"

namespace iimodmanager {

CacheCommands::CacheCommands(ModManCliApplication &app)
    : CommandCategory(app)
{}

void CacheCommands::addArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("cache", "Category: Manage the cache of downloaded mods");
}

void CacheCommands::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("command", "Command to be executed (list)", "list||help");
}

bool CacheCommands::parseCommands(QCommandLineParser &parser, const QStringList &args, bool isHelpSet, const QString command) const
{
    if (command == "list")
    {
        CacheListCommand listCommand(app_);
        listCommand.parse(parser, args, isHelpSet);
        return true;
    }
    return false;
}


}  // namespace iimodmanager
