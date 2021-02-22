#include "configlistcommand.h"

#include <QTextStream>

namespace iimodmanager {

ConfigListCommand::ConfigListCommand(ModManCliApplication &app)
    : Command(app)
{}

void ConfigListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("list", "Command: List all config values");
}

void ConfigListCommand::execute(QCommandLineParser &parser, const QStringList &args) const
{
    Q_UNUSED(parser);
    Q_UNUSED(args);

    QTextStream cout(stdout);
    cout << "core.cachePath=" << app_.config().cachePath() << endl;
    cout << "core.installPath=" << app_.config().installPath() << endl;
}

}  // namespace iimodmanager
