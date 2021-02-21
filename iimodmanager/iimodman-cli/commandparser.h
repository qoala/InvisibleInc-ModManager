#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QStringList>

namespace iimodmanager {

class CommandParser
{
public:
    CommandParser(ModManCliApplication &app);

    void parse(const QStringList &args);

private:
    ModManCliApplication &app_;
    QCommandLineParser parser_;

    void addTerminalArgs();
};

}  // namespace iimodmanager

#endif // COMMANDPARSER_H
