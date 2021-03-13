#include "commandcategory.h"

#include <QTextStream>

namespace iimodmanager {

CommandCategory::~CommandCategory()
{}

void CommandCategory::parse(QCommandLineParser &parser, const QStringList &args, bool isHelpSet) const
{
    addArgs(parser);

    if (args.size() < 2)
    {
        addTerminalArgs(parser);
        if (isHelpSet)
        {
            parser.showHelp(EXIT_SUCCESS);
        }
        else
        {
            QTextStream cerr(stderr);
            cerr << app_.applicationName() << ": Missing command" << Qt::endl;
            parser.showHelp(EXIT_FAILURE);
        }
    }

    QString command = args.at(1);
    if (command == "help")
    {
        addTerminalArgs(parser);
        parser.showHelp(EXIT_SUCCESS);
    }
    else if (!parseCommands(parser, args, isHelpSet, command))
    {
        addTerminalArgs(parser);
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Unrecognized command: " << command << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }
}

CommandCategory::CommandCategory(ModManCliApplication &app)
    :app_(app)
{}

}  // namespace iimodmanager
