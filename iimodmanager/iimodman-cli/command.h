#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>

class QCommandLineParser;
template <typename T> class QFuture;


namespace iimodmanager {

class ModManCliApplication;


class Command : public QObject
{
public:
    virtual ~Command();

    virtual void addTerminalArgs(QCommandLineParser &parser) const = 0;
    virtual void parse(QCommandLineParser &parser, const QStringList &args) = 0;
    virtual QFuture<void> execute() = 0;

protected:
    ModManCliApplication &app_;

    Command(ModManCliApplication &app);
};

}  // namespace iimodmanager

#endif // COMMAND_H
