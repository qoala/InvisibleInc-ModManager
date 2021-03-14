#ifndef IIMODMANAGER_STEAMAPICOMMANDS_H
#define IIMODMANAGER_STEAMAPICOMMANDS_H

#include "commandcategory.h"

namespace iimodmanager {

class SteamAPICommands : public CommandCategory
{
public:
    SteamAPICommands(ModManCliApplication &app);

    // CommandCategory interface
protected:
    void addArgs(QCommandLineParser &parser) const;
    void addTerminalArgs(QCommandLineParser &parser) const;
    Command *parseCommands(const QString command) const;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_STEAMAPICOMMANDS_H
