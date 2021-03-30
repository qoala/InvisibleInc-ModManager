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
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();

private:
    UpdateModsImpl *impl;
    QString modId;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_STEAMAPIMODDOWNLOADCOMMAND_H
