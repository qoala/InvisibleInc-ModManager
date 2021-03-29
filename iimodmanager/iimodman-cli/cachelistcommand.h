#ifndef CACHELISTCOMMAND_H
#define CACHELISTCOMMAND_H

#include "command.h"

#include <modcache.h>
#include <QTextStream>

namespace iimodmanager {

class CacheListCommand : public Command
{
public:
    CacheListCommand(ModManCliApplication &app);

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
    enum VersionSetting
    {
        NONE,
        LATEST,
        ALL,
    };
    OutputFormat format;
    VersionSetting versionSetting;
    qsizetype maxWidth;

    void writeSpecMod(QTextStream &out, const CachedMod &mod);
    void writeTextMod(QTextStream &out, const CachedMod &mod);
};

}  // namespace iimodmanager

#endif // CACHELISTCOMMAND_H
