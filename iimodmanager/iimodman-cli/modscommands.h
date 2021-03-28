#ifndef IIMODMANAGER_MODSCOMMANDS_H
#define IIMODMANAGER_MODSCOMMANDS_H

#include "commandcategory.h"

namespace iimodmanager {

class ModsCommands : public CommandCategory
{
public:
    ModsCommands(ModManCliApplication &app);

    // CommandCategory interface
protected:
    void addArgs(QCommandLineParser &parser) const;
    void addTerminalArgs(QCommandLineParser &parser) const;
    Command *parseCommands(const QString command) const;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_MODSCOMMANDS_H
