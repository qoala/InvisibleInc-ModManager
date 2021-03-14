#ifndef IIMODMANAGER_STEAMAPIMODINFOCOMMAND_H
#define IIMODMANAGER_STEAMAPIMODINFOCOMMAND_H

#include "command.h"

namespace iimodmanager {

class SteamAPIModInfoCommand : public Command
{
public:
    SteamAPIModInfoCommand(ModManCliApplication &app);

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_STEAMAPIMODINFOCOMMAND_H
