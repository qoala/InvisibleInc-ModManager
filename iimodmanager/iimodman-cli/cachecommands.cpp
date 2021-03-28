#include "cacheaddcommand.h"
#include "cachecommands.h"
#include "cachelistcommand.h"
#include "cacheupdatecommand.h"

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
    parser.addPositionalArgument("command", "Command to be executed (add|list|update)", "add|list|update|help");
}

Command *CacheCommands::parseCommands(const QString command) const
{
    if (command == "add")
    {
        return new CacheAddCommand(app_);
    }
    if (command == "list")
    {
        return new CacheListCommand(app_);
    }
    else if (command == "update")
    {
        return new CacheUpdateCommand(app_);
    }
    return nullptr;
}


}  // namespace iimodmanager
