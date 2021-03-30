#include "modinfo.h"
#include "modcache.h"
#include "moddownloader.h"
#include "steamapimoddownloadcommand.h"
#include "updatemodsimpl.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QDir>
#include <QFutureWatcher>
#include <QPromise>

namespace iimodmanager {

SteamAPIDownloadModCommand::SteamAPIDownloadModCommand(ModManCliApplication &app)
    : Command(app)
{}

void SteamAPIDownloadModCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("moddl", "Command: Download the latest version of the specified mod");
    parser.addOptions({
                          {{"id", "steam-id"}, "REQUIRED: Steam workshop ID (e.g. '2151835746')", "id"},
                          {{"f","force"}, "Overwrite even if the latest version is already downloaded."},
                      });
}

void SteamAPIDownloadModCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);
    if (!parser.isSet("steam-id"))
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing steam-id" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    impl = new UpdateModsImpl(app_, this);
    impl->setMissingCacheAction(CACHE_ADD);
    impl->setAlreadyLatestVersionBehavior(parser.isSet("force") ? LATEST_FORCE : LATEST_SKIP);
    impl->setConfirmBeforeDownloading(false);

    modId = "workshop-" + parser.value("steam-id");

}

QFuture<void> SteamAPIDownloadModCommand::execute()
{
    QStringList modIds(modId);
    impl->start(modIds);

    return QtFuture::connect(impl, &UpdateModsImpl::finished);
}


} // namespace iimodmanager
