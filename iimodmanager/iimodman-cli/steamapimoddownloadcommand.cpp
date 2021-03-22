#include "moddownloader.h"
#include "steamapimoddownloadcommand.h"

#include <QFutureWatcher>
#include <QPromise>

namespace iimodmanager {

SteamAPIDownloadModCommand::SteamAPIDownloadModCommand(ModManCliApplication &app)
    : Command(app)
{}

void SteamAPIDownloadModCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("moddl", "Command: Download the latest version of the specified mod");
    parser.addPositionalArgument("id", "Steam workshop ID (e.g. '2151835746')", "[id]");
}

QFuture<void> SteamAPIDownloadModCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    if (args.size() < 3)
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing worskhop ID" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    const QString workshopId = args.at(2);

    ModDownloader *downloader = new ModDownloader(app_.config(), this);

    ModInfoCall *infoCall = downloader->modInfoCall();
    ModDownloadCall *downloadCall = downloader->modDownloadCall();

    infoCall->start(workshopId);

    connect(infoCall, &ModInfoCall::finished, downloadCall, [infoCall, downloadCall]
    {
       downloadCall->start(infoCall->result());
    });

    return QtFuture::connect(downloadCall, &ModDownloadCall::finished);
}


} // namespace iimodmanager
