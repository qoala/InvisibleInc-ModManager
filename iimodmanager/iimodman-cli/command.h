#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>

class QCommandLineParser;


namespace iimodmanager {

class ModManCliApplication;


class Command : public QObject
{
    Q_OBJECT

public:
    virtual ~Command();

    virtual void addTerminalArgs(QCommandLineParser &parser) const = 0;
    virtual void parse(QCommandLineParser &parser, const QStringList &args) = 0;
    virtual void execute() = 0;

signals:
    void finished();

protected:
    ModManCliApplication &app_;

    Command(ModManCliApplication &app);
};

}  // namespace iimodmanager

#endif // COMMAND_H
