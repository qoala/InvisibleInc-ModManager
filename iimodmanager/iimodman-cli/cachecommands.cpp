#include "cacheaddcommand.h"
#include "cacheaddinstalledcommand.h"
#include "cachecommands.h"
#include "cachelistcommand.h"
#include "cacheupdatecommand.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>

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
    parser.addPositionalArgument("command", "Command to be executed (add|add-installed|list|update)", "add|add-installed|list|update|help");
}

Command *CacheCommands::parseCommands(const QString command) const
{
    if (command == "add")
    {
        return new CacheAddCommand(app_);
    }
    if (command == "add-installed")
    {
        return new CacheAddInstalledCommand(app_);
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
