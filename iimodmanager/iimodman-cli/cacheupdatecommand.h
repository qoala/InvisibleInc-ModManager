#ifndef IIMODMANAGER_CACHEUPDATECOMMAND_H
#define IIMODMANAGER_CACHEUPDATECOMMAND_H

#include "command.h"

namespace iimodmanager {

class UpdateModsImpl;

class CacheUpdateCommand : public Command
{
public:
    CacheUpdateCommand(ModManCliApplication &app);

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args);

private:
    UpdateModsImpl *impl;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_CACHEUPDATECOMMAND_H
