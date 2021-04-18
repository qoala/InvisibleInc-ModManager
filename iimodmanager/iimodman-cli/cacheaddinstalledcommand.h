#ifndef IIMODMANAGER_CACHEADDINSTALLEDCOMMAND_H
#define IIMODMANAGER_CACHEADDINSTALLEDCOMMAND_H

#include "command.h"

namespace iimodmanager {

class ConfirmationPrompt;
class InstalledMod;
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
    ConfirmationPrompt *prompt;
    //! Mods where the current version isn't cached. (May have a presumed newer version in the cache)
    QList<InstalledMod> uncachedMods;

    QStringList modIds;

    void updateFinished();
    void startCopy();
    bool copyMod(const InstalledMod &);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_CACHEADDINSTALLEDCOMMAND_H
