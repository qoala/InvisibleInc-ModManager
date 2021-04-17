#ifndef IIMODMANAGER_CACHEADDINSTALLEDCOMMAND_H
#define IIMODMANAGER_CACHEADDINSTALLEDCOMMAND_H

#include "command.h"

namespace iimodmanager {

class ModCache;
class ModList;
class UpdateModsImpl;

class CacheAddInstalledCommand : public Command
{
public:
    CacheAddInstalledCommand(ModManCliApplication &app);

    // Command interface
public:
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();

private:
    ModCache *cache;
    ModList *modList;
    UpdateModsImpl *updateImpl;

    QStringList modIds;

    void updateFinished();
};

} // namespace iimodmanager

#endif // IIMODMANAGER_CACHEADDINSTALLEDCOMMAND_H
