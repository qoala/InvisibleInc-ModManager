#include "modmancliapplication.h"
#include "modscommands.h"
#include "modsinstallcommand.h"
#include "modslistcommand.h"

#include <QCommandLineParser>

namespace iimodmanager {

ModsCommands::ModsCommands(ModManCliApplication &app)
    : CommandCategory(app)
{}

void ModsCommands::addArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("mods", "Category: Manage mods currently installed to the II mods/ folder");
}

void ModsCommands::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("command", "Command to be executed (install|list)", "install|list|help");
}

Command *ModsCommands::parseCommands(const QString command) const
{
    if (command == "install")
    {
        return new ModsInstallCommand(app_);
    }
    else if (command == "list")
    {
        return new ModsListCommand(app_);
    }
    return nullptr;
}

} // namespace iimodmanager
