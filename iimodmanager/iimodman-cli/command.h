#ifndef COMMAND_H
#define COMMAND_H

#include "modmancliapplication.h"

#include <QCommandLineParser>


namespace iimodmanager {

class Command
{
public:
    virtual ~Command();

    void parse(QCommandLineParser &parser, const QStringList &args, bool isHelpSet) const;
protected:
    ModManCliApplication &app_;

    Command(ModManCliApplication &app);

    virtual void addTerminalArgs(QCommandLineParser &parser) const = 0;
    virtual void execute(const QCommandLineParser &parser, const QStringList &args) const = 0;
};

}  // namespace iimodmanager

#endif // COMMAND_H
