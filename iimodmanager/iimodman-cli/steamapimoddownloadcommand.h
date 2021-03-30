#ifndef IIMODMANAGER_STEAMAPIMODDOWNLOADCOMMAND_H
#define IIMODMANAGER_STEAMAPIMODDOWNLOADCOMMAND_H

#include "command.h"


namespace iimodmanager {

class UpdateModsImpl;

class SteamAPIDownloadModCommand : public Command
{
public:
    SteamAPIDownloadModCommand(ModManCliApplication &app);

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args);

private:
    UpdateModsImpl *impl;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_STEAMAPIMODDOWNLOADCOMMAND_H
