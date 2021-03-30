#include "cachelistcommand.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QTextStream>
#include <QTimer>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

CacheListCommand::CacheListCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("list", "Command: List all downloaded mods in the cache");
    parser.addOptions({
                          {{"a", "all"}, "List all versions of each mod."},
                          {"hash", "Print mod version hashes."},
                          {"spec", "Format output as a modspec."},
                      });
}

void CacheListCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);

    format = TEXT;
    versionSetting = parser.isSet("all") ? ALL : LATEST;
    includeHashes = parser.isSet("hash");
    if (parser.isSet("spec"))
    {
        format = MODSPEC;
        versionSetting = NONE;
        includeHashes = false;
    }
}

void CacheListCommand::execute()
{
    ModCache cache(app_.config());
    if (versionSetting == ALL)
    {
        cache.refresh(ModCache::FULL);
        ModList modList(app_.config(), &cache, nullptr);
        modList.refresh(ModList::FULL);
    }
    else
    {
        cache.refresh(ModCache::LATEST_ONLY);
    }

    if (format == TEXT)
    {
        maxWidth = 0;
        for (auto mod : cache.mods()) {
        const qsizetype width = mod.id().size() + mod.info().name().size() + 3;
        if (width > maxWidth)
            maxWidth = width;
        }

        QTextStream cout(stdout);
        cout.setFieldAlignment(QTextStream::AlignLeft);
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

    QTimer::singleShot(0, this, &Command::finished);
}

void CacheListCommand::writeSpecMod(QTextStream &cout, const CachedMod &mod)
{
    cout << mod.id() << ":" << mod.info().name() << Qt::endl;
}

void CacheListCommand::writeTextMod(QTextStream &cout, const CachedMod &mod)
{
    const ModInfo &info = mod.info();

    cout << qSetFieldWidth(maxWidth) <<  info.toString() << qSetFieldWidth(0);

    if (versionSetting == LATEST)
    {
        cout << "  ";
        cout << qSetFieldWidth(29);
        cout << (mod.downloaded() ? mod.latestVersion()->toString() : "(not downloaded)");
        cout << qSetFieldWidth(0);
        if (includeHashes && mod.downloaded())
            cout << ' ' << mod.latestVersion()->hash();
    }
    else if (versionSetting == ALL)
    {
        if (mod.downloaded())
        {
            const QList<CachedVersion> &versions = mod.versions();
            const qsizetype fieldWidth = std::max<qsizetype>(maxWidth - 2, 0);
            for (auto it = versions.rbegin(); it != versions.rend() ; ++it)
            {
                cout << Qt::endl << "  " << qSetFieldWidth(fieldWidth) << it->toString();
                cout << qSetFieldWidth(20);
                if (it->installed())
                    cout << "  (installed)";
                else
                    cout << ' ';
                cout << qSetFieldWidth(0);
                if (includeHashes)
                    cout << ' ' << it->hash();
            }
        }
        else
        {
            cout << Qt::endl << QString(maxWidth, ' ') << "  (not downloaded)";
        }
    }

    cout << Qt::endl;
}

}  // namespace iimodmanager
