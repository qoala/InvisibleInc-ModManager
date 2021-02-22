#ifndef CONFIGSETCOMMAND_H
#define CONFIGSETCOMMAND_H

#include "command.h"


namespace iimodmanager {

class ConfigSetCommand : public Command
{
public:
    ConfigSetCommand(ModManCliApplication &app);

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    void execute(QCommandLineParser &parser, const QStringList &args) const;
};

}  // namespace iimodmanager

#endif // CONFIGSETCOMMAND_H
