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
    cache.refresh();

    QTextStream cout(stdout);
    for (auto mod : cache.mods()) {
        cout << mod.id() << ":" << mod.info().name() << Qt::endl;
    }

    QPromise<void> promise;
    promise.finish();
    return promise.future();
}

}  // namespace iimodmanager
