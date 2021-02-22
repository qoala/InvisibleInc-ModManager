#ifndef CACHELISTCOMMAND_H
#define CACHELISTCOMMAND_H

#include "command.h"

namespace iimodmanager {

class CacheListCommand : public Command
{
public:
    CacheListCommand(ModManCliApplication &app);

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    void execute(QCommandLineParser &parser, const QStringList &args) const;
};

}  // namespace iimodmanager

#endif // CACHELISTCOMMAND_H
