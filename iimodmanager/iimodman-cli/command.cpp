#include "command.h"

namespace iimodmanager {

Command::~Command()
{}

QFuture<void> Command::execute(QCommandLineParser &parser, const QStringList &args, bool isHelpSet)
{
    addTerminalArgs(parser);

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
