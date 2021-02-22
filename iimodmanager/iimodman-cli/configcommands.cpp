#include "configcommands.h"
#include "configgetcommand.h"
#include "configlistcommand.h"
#include "configsetcommand.h"

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

bool ConfigCommands::parseCommands(QCommandLineParser &parser, const QStringList &args, bool isHelpSet, const QString command) const
{
    if (command == "list")
    {
        ConfigListCommand listCommand(app_);
        listCommand.parse(parser, args, isHelpSet);
        return true;
    }
    else if (command == "get")
    {
        ConfigGetCommand getCommand(app_);
        getCommand.parse(parser, args, isHelpSet);
        return true;
    }
    else if (command == "set")
    {
        ConfigSetCommand setCommand(app_);
        setCommand.parse(parser, args, isHelpSet);
        return true;
    }
    return false;
}

}  // namespace iimodmanager
