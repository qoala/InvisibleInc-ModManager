#include "cacheupdatecommand.h"
#include "updatemodsimpl.h"

namespace iimodmanager {

CacheUpdateCommand::CacheUpdateCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheUpdateCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("update", "Command: Update downloaded mods currently in the cache");
    parser.addOptions({
                    {{"m", "mod-id"}, "mod ID (e.g. 'workshop-2151835746')", "id"},
                    {{"f","force"}, "Overwrite even if the latest version is already downloaded."},
                });
}

QFuture<void> CacheUpdateCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);
    QStringList modIds;

    const bool isForceSet = parser.isSet("force");

    if (parser.isSet("mod-id"))
    {
        modIds.append(parser.value("mod-id"));
    }
    else
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing mod-id" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    impl = new UpdateModsImpl(app_, this);
    impl->setAlreadyLatestVersionBehavior(isForceSet ? FORCE_UPDATE : SKIP);

    impl->start(modIds);

    return QtFuture::connect(impl, &UpdateModsImpl::finished);
}

} // namespace iimodmanager
