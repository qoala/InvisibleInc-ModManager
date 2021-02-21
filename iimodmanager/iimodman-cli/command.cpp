#include "command.h"

namespace iimodmanager {

Command::~Command()
{}

void Command::parse(QCommandLineParser &parser, const QStringList &args, bool isHelpSet) const
{
    addTerminalArgs(parser);

    if (isHelpSet || (args.size() >= 3 && args.at(2) == "help"))
    {
        parser.showHelp(EXIT_SUCCESS);
    }

    execute(parser, args);
}

Command::Command(ModManCliApplication &app)
    : app_(app)
{}

}  // namespace iimodmanager
