#include "command.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QFuture>

namespace iimodmanager {

Command::~Command()
{}

void Command::parse(QCommandLineParser &parser, const QStringList &arguments)
{
    addTerminalArgs(parser);
    parser.process(arguments);
}

QFuture<void> Command::execute(QCommandLineParser &parser, const QStringList &args, bool isHelpSet)
{
    if (isHelpSet || (args.size() >= 3 && args.at(2) == "help"))
    {
        parser.showHelp(EXIT_SUCCESS);
    }

    return executeCommand(parser, args);
}

Command::Command(ModManCliApplication &app)
    : QObject(&app), app_(app)
{}

}  // namespace iimodmanager
