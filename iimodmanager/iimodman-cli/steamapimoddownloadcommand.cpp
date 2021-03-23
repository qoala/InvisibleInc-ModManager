#include "mod.h"
#include "moddownloader.h"
#include "steamapimoddownloadcommand.h"

#include <QDir>
#include <QFutureWatcher>
#include <QPromise>

namespace iimodmanager {

SteamAPIDownloadModCommand::SteamAPIDownloadModCommand(ModManCliApplication &app)
    : Command(app), forceOption(QStringList() << "f" << "force", "Overwrite even if the latest version is already downloaded.")
{}

void SteamAPIDownloadModCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("moddl", "Command: Download the latest version of the specified mod");
    parser.addPositionalArgument("id", "Steam workshop ID (e.g. '2151835746')", "[id]");
    parser.addOption(forceOption);
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
    const bool isForceSet = parser.isSet(forceOption);

    ModDownloader *downloader = new ModDownloader(app_.config(), this);

    ModInfoCall *infoCall = downloader->modInfoCall();
    ModDownloadCall *downloadCall = downloader->modDownloadCall();

    infoCall->start(workshopId);

    connect(infoCall, &ModInfoCall::finished, this, [this, isForceSet, infoCall, downloadCall]
    {
        const SteamModInfo info = infoCall->result();
        infoCall->deleteLater();

        QDir cacheDir(app_.config().cachePath());
        QDir modDir(cacheDir.absoluteFilePath(QString("workshop-%1").arg(info.id)));
        QDir modVersionDir(modDir.absoluteFilePath(info.lastUpdated.toString("yyyy-MM-ddTHH-mm-ss")));

        if (!isForceSet && modVersionDir.exists() && modVersionDir.exists("modinfo.txt"))
        {
            QTextStream cerr(stderr);
            QFile modInfoFile = QFile(modVersionDir.absoluteFilePath("modinfo.txt"));
            Mod mod = Mod::readModInfo(modInfoFile, info.modId(), CACHED);
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
        Mod mod = Mod::readModInfo(modInfoFile, downloadCall->modInfo().modId(), CACHED);
        cerr << mod.toString() << " updated" << Qt::endl;

        downloadCall->deleteLater();
        result.finish();
    });
    return result.future();
}


} // namespace iimodmanager
