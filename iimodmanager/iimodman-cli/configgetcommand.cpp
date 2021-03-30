#include "configgetcommand.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QPromise>
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

void ConfigGetCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    if (args.size() < 3)
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing config key" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    key = args.at(2);
}

QFuture<void> ConfigGetCommand::execute()
{
    QTextStream cout(stdout);
    if (key == "core.cachePath")
    {
        cout << app_.config().cachePath() << Qt::endl;
    }
    else if (key == "core.installPath")
    {
        cout << app_.config().installPath() << Qt::endl;
    }
    else if (key == "steam.apiKey")
    {
        cout << app_.config().steamApiKey() << Qt::endl;
    }
    else
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Unknown config key (try 'config list'): " << key << Qt::endl;
        app_.exit(EXIT_FAILURE);
    }

    QPromise<void> promise;
    promise.finish();
    return promise.future();
}

}  // namespace iimodmanager
