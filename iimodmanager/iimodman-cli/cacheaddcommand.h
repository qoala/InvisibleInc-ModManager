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
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args);

private:
    AddModsImpl *impl;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_CACHEADDCOMMAND_H
