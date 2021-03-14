#include "commandparser.h"
#include "configcommands.h"
#include "commandcategory.h"
#include "cachecommands.h"
#include "steamapicommands.h"

#include <QTextStream>

namespace iimodmanager {

CommandParser::CommandParser(ModManCliApplication &app) : app_(app) {}

void CommandParser::addTerminalArgs() {
    parser_.addPositionalArgument("category", "Category of the command to be executed (cache|config|steamapi)", "(cache|config|steamapi)");
    parser_.addPositionalArgument("command", "Command to be executed", "[command]|help");
}

QFuture<void> CommandParser::parse(const QStringList &arguments)
{
    parser_.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    QCommandLineOption help = parser_.addHelpOption();

    QTextStream cerr(stderr);

    if (!parser_.parse(arguments))
    {
        cerr << app_.applicationName() << ": " << parser_.errorText() << Qt::endl;
        parser_.showHelp(EXIT_FAILURE);
    }
    QStringList args = parser_.positionalArguments();
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

    return command->execute(parser_, args, parser_.isSet(help));
}

}  // namespace iimodmanager
