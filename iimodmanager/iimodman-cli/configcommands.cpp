#include "configcommands.h"
#include "configlistcommand.h"

namespace iimodmanager {

ConfigCommands::ConfigCommands(ModManCliApplication &app)
    : CommandCategory(app)
{}

void ConfigCommands::addArgs(QCommandLineParser &parser) const {
    parser.addPositionalArgument("", "", "config");
}

void ConfigCommands::addTerminalArgs(QCommandLineParser &parser) const {
    parser.addPositionalArgument("command", "Command to be executed (list)", "list|help");
}

bool ConfigCommands::parseCommands(QCommandLineParser &parser, const QStringList &args, bool isHelpSet, const QString command) const
{
    if (command == "list") {
        ConfigListCommand listCommand(app_);
        listCommand.parse(parser, args, isHelpSet);
        return true;
    }
    return false;
}

}  // namespace iimodmanager
