#ifndef COMMANDCATEGORY_H
#define COMMANDCATEGORY_H

#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QString>

namespace iimodmanager {

class CommandCategory
{
public:
    virtual ~CommandCategory();

    void parse(QCommandLineParser &parser, const QStringList &args, bool isHelpSet) const;
protected:
    ModManCliApplication &app_;

    CommandCategory(ModManCliApplication &app);

    virtual void addArgs(QCommandLineParser &parser) const = 0;
    virtual void addTerminalArgs(QCommandLineParser &parser) const = 0;
    virtual bool parseCommands(QCommandLineParser &parser, const QStringList &args, bool isHelpSet, const QString command) const = 0;
};

}  // namespace iimodmanager

#endif // COMMANDCATEGORY_H
