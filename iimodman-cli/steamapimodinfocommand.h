#ifndef IIMODMANAGER_STEAMAPIMODINFOCOMMAND_H
#define IIMODMANAGER_STEAMAPIMODINFOCOMMAND_H

#include "command.h"

namespace iimodmanager {

class SteamAPIModInfoCommand : public Command
{
public:
    SteamAPIModInfoCommand(ModManCliApplication &app);

    // Command interface
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();

private:
    QString workshopId;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_STEAMAPIMODINFOCOMMAND_H
