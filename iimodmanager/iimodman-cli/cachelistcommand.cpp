#include "cachelistcommand.h"

#include <QPromise>
#include <QTextStream>
#include <modinfo.h>
#include <modcache.h>

namespace iimodmanager {

CacheListCommand::CacheListCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("list", "Command: List all downloaded mods in the cache");
}

QFuture<void> CacheListCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(parser);
    Q_UNUSED(args);

    ModCache cache(app_.config());
    cache.refresh(REFRESH_LATEST);

    int maxWidth = 0;
    for (auto mod : cache.mods()) {
        const int width = mod.id().size() + mod.info().name().size();
        if (width > maxWidth)
            maxWidth = width;
    }

    QTextStream cout(stdout);
    for (auto mod : cache.mods()) {
        const QString &modId = mod.id();
        const QString &modName = mod.info().name();
        const int width = modId.size() + modName.size();
        cout << modId << ":" << modName;
        cout << QString(maxWidth - width, ' ') << "  ::";
        cout << (mod.downloaded() ? mod.latest().toString() : "(not downloaded)" );
        cout << Qt::endl;
    }

    QPromise<void> promise;
    promise.finish();
    return promise.future();
}

}  // namespace iimodmanager
