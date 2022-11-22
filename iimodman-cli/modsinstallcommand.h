#ifndef IIMODMANAGER_MODSINSTALLCOMMAND_H
#define IIMODMANAGER_MODSINSTALLCOMMAND_H

#include "command.h"

#include <QList>
#include <optional>

namespace iimodmanager {

class ConfirmationPrompt;
class ModCache;
class ModList;
class SpecMod;

class ModsInstallCommand : public Command
{
public:
    ModsInstallCommand(ModManCliApplication &app);

    // Command interface
public:
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();

private:
    ModCache *cache;
    ModList *modList;
    QStringList modIds;

    ConfirmationPrompt *prompt;
    QList<SpecMod> specMods;

    void doInstalls();
    std::optional<const SpecMod> specForLatest(const QString &modId);
    bool installMod(const SpecMod &specMod);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_MODSINSTALLCOMMAND_H
