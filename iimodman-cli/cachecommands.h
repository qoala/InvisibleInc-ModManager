#ifndef CACHECOMMANDS_H
#define CACHECOMMANDS_H

#include "commandcategory.h"

namespace iimodmanager {

class CacheCommands : public CommandCategory
{
public:
    CacheCommands(ModManCliApplication &app);

    // CommandCategory interface
protected:
    void addArgs(QCommandLineParser &parser) const;
    void addTerminalArgs(QCommandLineParser &parser) const;
    Command *parseCommands(const QString command) const;
};

}  // namespace iimodmanager

#endif // CACHECOMMANDS_H
