#include "modinfo.h"
#include "modcache.h"
#include "moddownloader.h"
#include "steamapimoddownloadcommand.h"
#include "updatemodsimpl.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QDir>

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
    impl->setVerb(UpdateModsImpl::VERB_DOWNLOAD);
    impl->setMissingCacheAction(UpdateModsImpl::CACHE_ADD);
    impl->setAlreadyLatestVersionBehavior(parser.isSet("force") ? UpdateModsImpl::LATEST_FORCE : UpdateModsImpl::LATEST_SKIP);
    impl->setConfirmBeforeDownloading(false);
    impl->setVerbose(true);

    modId = "workshop-" + parser.value("steam-id");

}

void SteamAPIDownloadModCommand::execute()
{
    QStringList modIds(modId);
    impl->start(modIds);

    connect(impl, &UpdateModsImpl::finished, this, &Command::finished);
}


} // namespace iimodmanager
