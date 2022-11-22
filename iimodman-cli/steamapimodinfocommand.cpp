#include "moddownloader.h"
#include "modmancliapplication.h"
#include "steamapimodinfocommand.h"

#include <QCommandLineParser>
#include <QTextStream>

namespace iimodmanager {

SteamAPIModInfoCommand::SteamAPIModInfoCommand(ModManCliApplication &app)
    : Command(app)
{}

void SteamAPIModInfoCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("modinfo", "Command: Retrieve mod info for the specified mod");
    parser.addOptions({
                          {{"id", "steam-id"}, "REQUIRED: Steam workshop ID (e.g. '2151835746')", "id"},
                      });
}

void SteamAPIModInfoCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);
    if (!parser.isSet("steam-id"))
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Missing steam-id" << Qt::endl;
        parser.showHelp(EXIT_FAILURE);
    }

    workshopId = parser.value("steam-id");
}

void SteamAPIModInfoCommand::execute()
{
    ModDownloader *downloader = new ModDownloader(app_.config(), this);
    ModInfoCall *call = downloader->fetchModInfo(workshopId);

    connect(call, &ModInfoCall::finished, this, [this, call]
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

        emit finished();
    });
}

} // namespace iimodmanager
