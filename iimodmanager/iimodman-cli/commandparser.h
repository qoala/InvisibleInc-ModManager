#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include "modmancliapplication.h"

#include <QCommandLineParser>


namespace iimodmanager {

class Command;


class CommandParser
{
public:
    CommandParser(ModManCliApplication &app);

    Command *parse(const QStringList &args);

private:
    ModManCliApplication &app_;
    QCommandLineParser parser_;

    void addTerminalArgs();
};

}  // namespace iimodmanager

#endif // COMMANDPARSER_H
