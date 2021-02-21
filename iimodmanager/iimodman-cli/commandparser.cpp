#include "commandparser.h"
#include "configcommands.h"
#include "commandcategory.h"

#include <QTextStream>

namespace iimodmanager {

CommandParser::CommandParser(ModManCliApplication &app) : app_(app) {}

void CommandParser::addTerminalArgs() {
    parser_.addPositionalArgument("category", "Category of the command to be executed (config)", "(config)");
    parser_.addPositionalArgument("command", "Command to be executed", "[command]|help");
}

void CommandParser::parse(const QStringList &arguments)
{
    parser_.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    QCommandLineOption help = parser_.addHelpOption();

    QTextStream cerr(stderr);

    if (!parser_.parse(arguments))
    {
        cerr << app_.applicationName() << ": " << parser_.errorText() << endl;
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
            cerr << app_.applicationName() << ": Missing command category" << endl;
            parser_.showHelp(EXIT_FAILURE);
        }
    }

    const QString category = args.first();
    if (category == "help")
    {
        addTerminalArgs();
        parser_.showHelp(EXIT_SUCCESS);
    }
    else if (category == "config")
    {
        ConfigCommands configCommands(app_);
        configCommands.parse(parser_, args, parser_.isSet(help));
    }
    else
    {
        addTerminalArgs();
        cerr << app_.applicationName() << ": Unrecognized command category: " << category << endl;
        parser_.showHelp(EXIT_FAILURE);
    }
}

}  // namespace iimodmanager
