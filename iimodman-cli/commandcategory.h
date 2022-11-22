#ifndef COMMANDCATEGORY_H
#define COMMANDCATEGORY_H

#include "command.h"


namespace iimodmanager {

class CommandCategory : public QObject
{
public:
    virtual ~CommandCategory();

    Command *parse(QCommandLineParser &parser, const QStringList &args, bool isHelpSet) const;
protected:
    ModManCliApplication &app_;

    CommandCategory(ModManCliApplication &app);

    virtual void addArgs(QCommandLineParser &parser) const = 0;
    virtual void addTerminalArgs(QCommandLineParser &parser) const = 0;
    virtual Command *parseCommands(const QString command) const = 0;
};

}  // namespace iimodmanager

#endif // COMMANDCATEGORY_H
