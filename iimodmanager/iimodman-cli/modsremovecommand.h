#ifndef IIMODMANAGER_MODSREMOVECOMMAND_H
#define IIMODMANAGER_MODSREMOVECOMMAND_H

#include "command.h"

#include <QList>

namespace iimodmanager {

class ConfirmationPrompt;
class InstalledMod;
class ModCache;
class ModList;

class ModsRemoveCommand : public Command
{
public:
    ModsRemoveCommand(ModManCliApplication &app);

    // Command interface
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();

private:
    ModCache *cache;
    ModList *modList;
    QStringList modIds;

    ConfirmationPrompt *prompt;
    QList<InstalledMod> mods;

    void doRemoves();
};

} // namespace iimodmanager

#endif // IIMODMANAGER_MODSREMOVECOMMAND_H
