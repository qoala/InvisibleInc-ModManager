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
    QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args);
};

}  // namespace iimodmanager

#endif // CONFIGGETCOMMAND_H
