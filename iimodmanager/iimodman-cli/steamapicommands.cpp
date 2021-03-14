#include "steamapicommands.h"
#include "steamapimodinfocommand.h"

namespace iimodmanager {

SteamAPICommands::SteamAPICommands(ModManCliApplication &app)
    : CommandCategory(app)
{}

void SteamAPICommands::addArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("steamapi", "Category: Steam API calls");
}

void SteamAPICommands::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("command", "Command to be executed (modinfo)", "modinfo|help");
}

Command *SteamAPICommands::parseCommands(const QString command) const
{
    if (command == "modinfo" || command == "mod-info")
    {
        return new SteamAPIModInfoCommand(app_);
    }
    return nullptr;
}

} // namespace iimodmanager
