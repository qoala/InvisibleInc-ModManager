#include "configlistcommand.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QDir>
#include <QTextStream>
#include <QTimer>

namespace iimodmanager {

ConfigListCommand::ConfigListCommand(ModManCliApplication &app)
    : Command(app)
{}

void ConfigListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("list", "Command: List all config values");
}

void ConfigListCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(parser);
    Q_UNUSED(args);
}

void ConfigListCommand::execute()
{
    QTextStream cout(stdout);
    cout << "core.cachePath=" << QDir::toNativeSeparators(app_.config().cachePath()) << Qt::endl;
    cout << "core.installPath=" << QDir::toNativeSeparators(app_.config().installPath()) << Qt::endl;
    cout << "core.localPath=" << QDir::toNativeSeparators(app_.config().localPath()) << Qt::endl;

    QTimer::singleShot(0, this, &Command::finished);
}

}  // namespace iimodmanager
