#include "configsetcommand.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
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

QFuture<void> ConfigSetCommand::execute()
{
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
        app_.exit(EXIT_FAILURE);
    }

    QPromise<void> promise;
    promise.finish();
    return promise.future();
}

}  // namespace iimodmanager
