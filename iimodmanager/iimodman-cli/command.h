#ifndef COMMAND_H
#define COMMAND_H

#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QFuture>


namespace iimodmanager {

class Command : public QObject
{
public:
    virtual ~Command();

    void parse(QCommandLineParser &parser, const QStringList &arguments);
    QFuture<void> execute(QCommandLineParser &parser, const QStringList &args, bool isHelpSet);
protected:
    ModManCliApplication &app_;

    Command(ModManCliApplication &app);

    virtual void addTerminalArgs(QCommandLineParser &parser) const = 0;
    virtual QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args) = 0;
};

}  // namespace iimodmanager

#endif // COMMAND_H
