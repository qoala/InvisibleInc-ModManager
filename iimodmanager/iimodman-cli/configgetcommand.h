#ifndef CONFIGGETCOMMAND_H
#define CONFIGGETCOMMAND_H

#include "command.h"


namespace iimodmanager {

class ConfigGetCommand : public Command
{
public:
    ConfigGetCommand(ModManCliApplication &app);

    // Command interface
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();

private:
    QString key;
};

}  // namespace iimodmanager

#endif // CONFIGGETCOMMAND_H
