#include "configlistcommand.h"

#include <QPromise>
#include <QTextStream>

namespace iimodmanager {

ConfigListCommand::ConfigListCommand(ModManCliApplication &app)
    : Command(app)
{}

void ConfigListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("list", "Command: List all config values");
}

QFuture<void> ConfigListCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(parser);
    Q_UNUSED(args);

    QTextStream cout(stdout);
    cout << "core.cachePath=" << app_.config().cachePath() << Qt::endl;
    cout << "core.installPath=" << app_.config().installPath() << Qt::endl;
    if (app_.config().steamApiKey().isEmpty())
    {
        cout << "steam.apiKey [unset]" << Qt::endl;
    }
    else
    {
        cout << "steam.apiKey [private] (Obtain explicitly with `config get steam.apiKey`)" << Qt::endl;
    }

    QPromise<void> promise;
    promise.finish();
    return promise.future();
}

}  // namespace iimodmanager
