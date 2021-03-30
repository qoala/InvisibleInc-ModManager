#ifndef IIMODMANAGER_CACHEADDCOMMAND_H
#define IIMODMANAGER_CACHEADDCOMMAND_H

#include "command.h"

namespace iimodmanager {

class AddModsImpl;

class CacheAddCommand : public Command
{
public:
    CacheAddCommand(ModManCliApplication &app);

    // Command interface
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();

private:
    AddModsImpl *impl;
    QStringList modIds;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_CACHEADDCOMMAND_H
