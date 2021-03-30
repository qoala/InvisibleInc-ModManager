#ifndef CONFIGSETCOMMAND_H
#define CONFIGSETCOMMAND_H

#include "command.h"


namespace iimodmanager {

class ConfigSetCommand : public Command
{
public:
    ConfigSetCommand(ModManCliApplication &app);

    // Command interface
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    QFuture<void> execute();

private:
    QString key;
    QString value;
};

}  // namespace iimodmanager

#endif // CONFIGSETCOMMAND_H
