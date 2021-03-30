#ifndef CONFIGLISTCOMMAND_H
#define CONFIGLISTCOMMAND_H

#include "command.h"


namespace iimodmanager {

class ConfigListCommand : public Command
{
public:
    ConfigListCommand(ModManCliApplication &app);

    // Command interface
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    QFuture<void> execute();
};

}  // namespace iimodmanager

#endif // CONFIGLISTCOMMAND_H
