#include "cachecommands.h"
#include "commandcategory.h"
#include "commandparser.h"
#include "configcommands.h"
#include "modscommands.h"
#include "steamapicommands.h"

#include <QTextStream>

namespace iimodmanager {

CommandParser::CommandParser(ModManCliApplication &app) : app_(app) {}

void CommandParser::addTerminalArgs() {
    parser_.addPositionalArgument("category", "Category of the command to be executed (cache|config|steamapi)", "(cache|config|steamapi)");
    parser_.addPositionalArgument("command", "Command to be executed", "[command]|help");
}

Command *CommandParser::parse(const QStringList &arguments)
{
    parser_.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    QCommandLineOption help = parser_.addHelpOption();

    QTextStream cerr(stderr);

    parser_.parse(arguments);
    const QStringList args = parser_.positionalArguments();
    if (args.isEmpty())
    {
        addTerminalArgs();
        if (parser_.isSet(help))
        {
            parser_.showHelp(EXIT_SUCCESS);
        }
        else
        {
            cerr << app_.applicationName() << ": Missing command category" << Qt::endl;
            parser_.showHelp(EXIT_FAILURE);
        }
    }

    Command *command;
    const QString category = args.first();
    if (category == "help")
    {
        addTerminalArgs();
        parser_.showHelp(EXIT_SUCCESS);
    }
    else if (category == "cache")
    {
        CacheCommands cacheCommands(app_);
        command = cacheCommands.parse(parser_, args, parser_.isSet(help));
    }
    else if (category == "config")
    {
        ConfigCommands configCommands(app_);
        command = configCommands.parse(parser_, args, parser_.isSet(help));
    }
    else if (category == "mods")
    {
        ModsCommands modsCommands(app_);
        command = modsCommands.parse(parser_, args, parser_.isSet(help));
    }
    else if (category == "steamapi" || category == "steam-api")
    {
        SteamAPICommands apiCommands(app_);
        command = apiCommands.parse(parser_, args, parser_.isSet(help));
    }
    else
    {
        addTerminalArgs();
        cerr << app_.applicationName() << ": Unrecognized command category: " << category << Qt::endl;
        parser_.showHelp(EXIT_FAILURE);
    }

    command->addTerminalArgs(parser_);
    if (parser_.isSet(help) || (args.size() >= 3 && args.at(2) == "help"))
        parser_.showHelp(EXIT_SUCCESS);
    parser_.process(arguments);
    command->parse(parser_, args);

    return command;
}

}  // namespace iimodmanager
