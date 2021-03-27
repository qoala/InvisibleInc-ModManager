#include "modinfo.h"
#include "modcache.h"
#include "moddownloader.h"
#include "steamapimoddownloadcommand.h"

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

QFuture<void> SteamAPIDownloadModCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);
    if (!parser.isSet("steam-id"))
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing steam-id" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    const QString workshopId = parser.value("steam-id");
    const bool isForceSet = parser.isSet("force");

    ModDownloader *downloader = new ModDownloader(app_.config(), this);

    ModInfoCall *infoCall = downloader->modInfoCall();
    ModDownloadCall *downloadCall = downloader->modDownloadCall();

    infoCall->start(workshopId);

    connect(infoCall, &ModInfoCall::finished, this, [this, isForceSet, infoCall, downloadCall]
    {
        const SteamModInfo info = infoCall->result();
        infoCall->deleteLater();

        QDir modVersionDir(ModCache::modPath(app_.config(), info.modId(), info.lastUpdated));

        if (!isForceSet && modVersionDir.exists() && modVersionDir.exists("modinfo.txt"))
        {
            QTextStream cerr(stderr);
            QFile modInfoFile = QFile(modVersionDir.absoluteFilePath("modinfo.txt"));
            ModInfo mod = ModInfo::readModInfo(modInfoFile, info.modId());
            cerr << mod.toString() << " already up to date" << Qt::endl;

            result.finish();
        }
        else
        {
            downloadCall->start(info);
        }
    });

    connect(downloadCall, &ModDownloadCall::finished, this, [this, downloadCall]
    {
        QDir modVersionDir(downloadCall->resultPath());

        QTextStream cerr(stderr);
        QFile modInfoFile = QFile(modVersionDir.absoluteFilePath("modinfo.txt"));
        ModInfo mod = ModInfo::readModInfo(modInfoFile, downloadCall->modInfo().modId());
        cerr << mod.toString() << " updated" << Qt::endl;

        downloadCall->deleteLater();
        result.finish();
    });
    return result.future();
}


} // namespace iimodmanager
