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
    bool parseCommands(QCommandLineParser &parser, const QStringList &args, bool isHelpSet, const QString command) const;
};

}  // namespace iimodmanager

#endif // CACHECOMMANDS_H
