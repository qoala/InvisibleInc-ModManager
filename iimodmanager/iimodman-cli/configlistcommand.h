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
    QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args);
};

}  // namespace iimodmanager

#endif // CONFIGLISTCOMMAND_H
