#ifndef IIMODMANAGER_MODSSYNCCOMMAND_H
#define IIMODMANAGER_MODSSYNCCOMMAND_H

#include "command.h"

#include <modspec.h>

namespace iimodmanager {

class ConfirmationPrompt;
class InstalledMod;
class ModCache;
class ModList;

class ModsSyncCommand : public Command
{
public:
    ModsSyncCommand(ModManCliApplication &app);

    // Command interface
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();
private:
    ModCache *cache;
    ModList *modList;
    bool isForceSet;
    QStringList specFileNames;

    // All ModSpec lines from input
    ModSpec inputSpec;
    // ModSpec with exact versions specified
    ModSpec targetSpec;
    // Mods that need to be newly installed
    QList<SpecMod> addedMods;
    // Mods that need to be updated
    QList<SpecMod> updatedMods;
    // Mods that need to be removed
    QList<InstalledMod> removedMods;

    ConfirmationPrompt *prompt;

    void doSync();

    bool readSpecFile(const QString &fileName);
    std::optional<SpecMod> makeInstallTarget(const SpecMod &inputSpec);
    bool checkInstalledMod(const InstalledMod &installedMod);
    bool removeMod(const InstalledMod &installedMod);
    bool installMod(const SpecMod &specMod);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_MODSSYNCCOMMAND_H
