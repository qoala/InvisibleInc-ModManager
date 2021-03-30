#ifndef IIMODMANAGER_CACHEUPDATECOMMAND_H
#define IIMODMANAGER_CACHEUPDATECOMMAND_H

#include "command.h"

namespace iimodmanager {

class ModCache;
class UpdateModsImpl;

class CacheUpdateCommand : public Command
{
public:
    CacheUpdateCommand(ModManCliApplication &app);

    // Command interface
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    QFuture<void> execute();

private:
    ModCache *cache;
    UpdateModsImpl *impl;
    QStringList modIds;
    bool inclDownloaded;
    bool inclAll;
};

} // namespace iimodmanager

#endif // IIMODMANAGER_CACHEUPDATECOMMAND_H
