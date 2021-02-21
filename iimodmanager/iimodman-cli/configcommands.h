#ifndef CONFIGCOMMANDS_H
#define CONFIGCOMMANDS_H

#include "commandcategory.h"

#include <QObject>

namespace iimodmanager {

class ConfigCommands : public CommandCategory
{
public:
    ConfigCommands(ModManCliApplication &app);

    // CommandCategory interface
protected:
    void addArgs(QCommandLineParser &parser) const;
    void addTerminalArgs(QCommandLineParser &parser) const;
    bool parseCommands(QCommandLineParser &parser, const QStringList &args, bool isHelpSet, const QString command) const;
};

}  // namespace iimodmanager

#endif // CONFIGCOMMANDS_H
