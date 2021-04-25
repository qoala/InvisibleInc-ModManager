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
    void addTerminalArgs(QCommandLineParser &parser) const;
    void parse(QCommandLineParser &parser, const QStringList &args);
    void execute();

private:
    enum OutputFormat
    {
        TEXT,
        MODSPEC,
        MODSPEC_VERSIONED,
    };
    OutputFormat format;
    bool includeHashes;
    int maxWidth;

    void writeTextMod(QTextStream &out, const InstalledMod &mod);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_MODSLISTCOMMAND_H
