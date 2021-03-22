#ifndef IIMODMANAGER_STEAMAPIMODDOWNLOADCOMMAND_H
#define IIMODMANAGER_STEAMAPIMODDOWNLOADCOMMAND_H

#include "command.h"

#include <QPromise>

namespace iimodmanager {

class SteamAPIDownloadModCommand : public Command
{
public:
    SteamAPIDownloadModCommand(ModManCliApplication &app);

private:
    QPromise<void> result_;

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_STEAMAPIMODDOWNLOADCOMMAND_H
