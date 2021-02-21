#ifndef CONFIGLISTCOMMAND_H
#define CONFIGLISTCOMMAND_H

#include "command.h"

namespace iimodmanager {

class ConfigListCommand : public Command
{
public:
    ConfigListCommand(ModManCliApplication &app);

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    void execute(const QCommandLineParser &parser, const QStringList &args) const;
};

}  // namespace iimodmanager

#endif // CONFIGLISTCOMMAND_H
