#include "configlistcommand.h"

#include <QTextStream>

namespace iimodmanager {

ConfigListCommand::ConfigListCommand(ModManCliApplication &app)
    : Command(app)
{}

void ConfigListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("", "", "list");
}

void ConfigListCommand::execute(const QCommandLineParser &parser, const QStringList &args) const
{
    Q_UNUSED(parser);
    Q_UNUSED(args);

    QTextStream cout(stdout);
    cout << "DownloadPath=" << app_.config().downloadPath() << endl;
    cout << "InstallPath=" << app_.config().installPath() << endl;
}

}  // namespace iimodmanager
