#include "moddownloader.h"
#include "steamapimodinfocommand.h"

#include <QTextStream>

namespace iimodmanager {

SteamAPIModInfoCommand::SteamAPIModInfoCommand(ModManCliApplication &app)
    : Command(app)
{}

void SteamAPIModInfoCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("modinfo", "Command: Retrieve mod info for the specified mod");
    parser.addPositionalArgument("id", "Steam workshop ID (e.g. '2151835746')", "[id]");
}

QFuture<void> SteamAPIModInfoCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    if (args.size() < 3)
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing worskhop ID" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    const QString workshopId = args.at(2);

    ModDownloader *downloader = new ModDownloader(app_.config(), this);
    ModInfoCall *call = downloader->fetchModInfo(workshopId);

    return QtFuture::connect(call, &ModInfoCall::finished)
            .then([call]
    {
        const SteamModInfo modInfo = call->result();
        call->deleteLater();

        QTextStream cout(stdout);
        cout << modInfo.title << " (workshop-" << modInfo.id << ")" << Qt::endl;
        cout << "Updated: " << modInfo.lastUpdated.toString() << Qt::endl;
        cout << "Download: " << modInfo.downloadUrl << Qt::endl;
        cout << "---" << Qt::endl;
        cout << modInfo.description << Qt::endl;
        cout << "---" << Qt::endl;
    });
}

} // namespace iimodmanager
