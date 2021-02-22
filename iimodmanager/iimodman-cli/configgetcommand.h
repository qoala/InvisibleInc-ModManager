#ifndef CONFIGGETCOMMAND_H
#define CONFIGGETCOMMAND_H

#include "command.h"


namespace iimodmanager {

class ConfigGetCommand : public Command
{
public:
    ConfigGetCommand(ModManCliApplication &app);

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    void execute(QCommandLineParser &parser, const QStringList &args) const;
};

}  // namespace iimodmanager

#endif // CONFIGGETCOMMAND_H
