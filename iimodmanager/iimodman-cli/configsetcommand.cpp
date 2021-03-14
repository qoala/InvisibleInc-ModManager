#include "configsetcommand.h"

#include <QPromise>
#include <QTextStream>

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

QFuture<void> ConfigSetCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
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

    QString key = args.at(2);
    QString value = args.at(3);

    if (key == "core.cachePath")
    {
        app_.config().setCachePath(value);
    }
    else if (key == "core.installPath")
    {
        app_.config().setInstallPath(value);
    }
    else if (key == "steam.apiKey")
    {
        app_.config().setSteamApiKey(value);
    }
    else
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Unknown config key: " << key << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    QPromise<void> promise;
    promise.finish();
    return promise.future();
}

}  // namespace iimodmanager
