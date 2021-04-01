#include "configsetcommand.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QTextStream>
#include <QTimer>

namespace iimodmanager {

ConfigSetCommand::ConfigSetCommand(ModManCliApplication &app)
    : Command(app)
{}

void ConfigSetCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("set", "Command: Set a single config value");
    parser.addPositionalArgument("key", "Config key to set", "[key]");
    parser.addPositionalArgument("value", "Config value to set", "[value]");
}

void ConfigSetCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    if (args.size() < 3)
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing config key" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }
    else if (args.size() < 4)
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing config value" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    key = args.at(2);
    value = args.at(3);

}

void ConfigSetCommand::execute()
{
    if (key == "core.cachePath")
    {
        app_.config().setCachePath(value);
    }
    else if (key == "core.installPath")
    {
        app_.config().setInstallPath(value);
    }
    else if (key == "core.localPath")
    {
        app_.config().setLocalPath(value);
    }
    else
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Unknown config key: " << key << Qt::endl;
        app_.exit(EXIT_FAILURE);
    }

    QTimer::singleShot(0, this, &Command::finished);
}

}  // namespace iimodmanager
