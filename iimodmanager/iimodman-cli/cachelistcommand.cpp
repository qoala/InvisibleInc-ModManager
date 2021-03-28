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
    parser.addOptions({
                    {{"a", "all"}, "List all versions of each mod."},
                    {"spec", "Format output as a modspec."},
                });
}

QFuture<void> CacheListCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);

    format = TEXT;
    versionSetting = parser.isSet("all") ? ALL : LATEST;
    if (parser.isSet("spec"))
    {
        format = MODSPEC;
        versionSetting = NONE;
    }

    ModCache cache(app_.config());
    cache.refresh(versionSetting == ALL ? ModCache::FULL : ModCache::LATEST_ONLY);

    if (format == TEXT)
    {
        maxWidth = 0;
        for (auto mod : cache.mods()) {
        const int width = mod.id().size() + mod.info().name().size();
        if (width > maxWidth)
            maxWidth = width;
        }

        QTextStream cout(stdout);
        for (auto mod : cache.mods()) {
            writeTextMod(cout, mod);
        }
    }
    else if (format == MODSPEC)
    {
        QTextStream cout(stdout);
        for (auto mod : cache.mods()) {
            writeSpecMod(cout, mod);
        }
    }

    QPromise<void> promise;
    promise.finish();
    return promise.future();
}

void CacheListCommand::writeSpecMod(QTextStream &cout, const CachedMod &mod)
{
    cout << mod.id() << ":" << mod.info().name() << Qt::endl;
}

void CacheListCommand::writeTextMod(QTextStream &cout, const CachedMod &mod)
{
    const ModInfo &info = mod.info();
    const int width = info.id().size() + info.name().size();

    cout << info.toString();
    cout << QString(maxWidth - width, ' ') << "  ::";

    if (versionSetting != NONE && !mod.downloaded())
    {
        cout << "(not downloaded)";
    }
    else if (versionSetting == LATEST)
    {
        cout << mod.latest()->toString();
    }
    else if (versionSetting == ALL)
    {
        const QList<CachedVersion> &versions = mod.versions();
        for (auto it = versions.rbegin(); it != versions.rend() ; ++it)
        {
            if (it != versions.rbegin())
                cout << ", ";
            cout << it->toString();
        }
    }

    cout << Qt::endl;
}

}  // namespace iimodmanager
