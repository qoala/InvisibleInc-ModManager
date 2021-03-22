#include "steamapicommands.h"
#include "steamapimoddownloadcommand.h"
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
    parser.addPositionalArgument("command", "Command to be executed (modinfo|moddl)", "modinfo|moddl|help");
}

Command *SteamAPICommands::parseCommands(const QString command) const
{
    if (command == "modinfo" || command == "mod-info")
    {
        return new SteamAPIModInfoCommand(app_);
    }
    if (command == "moddl" || command == "moddownload" || command == "mod-dl" || command == "mod-download")
    {
        return new SteamAPIDownloadModCommand(app_);
    }
    return nullptr;
}

} // namespace iimodmanager
