#ifndef CACHELISTCOMMAND_H
#define CACHELISTCOMMAND_H

#include "command.h"

class QTextStream;


namespace iimodmanager {

class CachedMod;


class CacheListCommand : public Command
{
public:
    CacheListCommand(ModManCliApplication &app);

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
    enum VersionSetting
    {
        NONE,
        LATEST,
        ALL,
    };
    OutputFormat format;
    VersionSetting versionSetting;
    bool includeHashes;
    qsizetype maxWidth;

    void writeTextMod(QTextStream &out, const CachedMod &mod);
};

}  // namespace iimodmanager

#endif // CACHELISTCOMMAND_H
