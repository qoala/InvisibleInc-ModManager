#include "configcommands.h"
#include "configgetcommand.h"
#include "configlistcommand.h"
#include "configsetcommand.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>

namespace iimodmanager {

ConfigCommands::ConfigCommands(ModManCliApplication &app)
    : CommandCategory(app)
{}

void ConfigCommands::addArgs(QCommandLineParser &parser) const {
    parser.addPositionalArgument("config", "Category: Configs for II Mod Manager");
}

void ConfigCommands::addTerminalArgs(QCommandLineParser &parser) const {
    parser.addPositionalArgument("command", "Command to be executed (list|get|set)", "list|get|set|help");
}

Command *ConfigCommands::parseCommands(const QString command) const
{
    if (command == "list")
    {
        return new ConfigListCommand(app_);
    }
    else if (command == "get")
    {
        return new ConfigGetCommand(app_);
    }
    else if (command == "set")
    {
        return new ConfigSetCommand(app_);
    }
    return nullptr;
}

}  // namespace iimodmanager
