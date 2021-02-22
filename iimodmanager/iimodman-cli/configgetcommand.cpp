#include "configgetcommand.h"

#include <QTextStream>

namespace iimodmanager {

ConfigGetCommand::ConfigGetCommand(ModManCliApplication &app)
    : Command(app)
{}

void ConfigGetCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("get", "Command: Retrieve a single config value");
    parser.addPositionalArgument("key", "Config key to retrieve", "[key]");
}

void ConfigGetCommand::execute(QCommandLineParser &parser, const QStringList &args) const
{
    if (args.size() < 3)
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing config key" << endl;
        parser.showHelp(EXIT_FAILURE);
    }

    QString key = args.at(2);
    QTextStream cout(stdout);
    if (key == "core.cachePath")
    {
        cout << app_.config().cachePath() << endl;
    }
    else if (key == "core.installPath")
    {
        cout << app_.config().installPath() << endl;
    }
    else
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Unknown config key: " << key << endl;
        parser.showHelp(EXIT_FAILURE);
    }
}

}  // namespace iimodmanager
