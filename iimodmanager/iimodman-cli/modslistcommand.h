#ifndef IIMODMANAGER_MODSLISTCOMMAND_H
#define IIMODMANAGER_MODSLISTCOMMAND_H

#include "command.h"

class QTextStream;


namespace iimodmanager {

class InstalledMod;


class ModsListCommand : public Command
{
public:
    ModsListCommand(ModManCliApplication &app);

    // Command interface
protected:
    void addTerminalArgs(QCommandLineParser &parser) const;
    QFuture<void> executeCommand(QCommandLineParser &parser, const QStringList &args);

private:
    enum OutputFormat
    {
        TEXT,
        MODSPEC,
    };
    OutputFormat format;
    bool includeHashes;
    int maxWidth;

    void writeSpecMod(QTextStream &out, const InstalledMod &mod);
    void writeTextMod(QTextStream &out, const InstalledMod &mod);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_MODSLISTCOMMAND_H
