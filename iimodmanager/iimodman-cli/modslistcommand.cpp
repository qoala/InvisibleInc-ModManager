#include "modmancliapplication.h"
#include "modslistcommand.h"

#include <QCommandLineParser>
#include <QTextStream>
#include <QTimer>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

ModsListCommand::ModsListCommand(ModManCliApplication &app)
    : Command(app)
{}

void ModsListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("list", "Command: List all installed mods");
    parser.addOptions({
                          {"hash", "Print mod version hashes."},
                          {"spec", "Format output as a modspec."},
                      });
}

void ModsListCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);

    format = parser.isSet("spec") ? MODSPEC : TEXT;
    includeHashes = parser.isSet("hash");
}

void ModsListCommand::execute()
{
    ModCache cache(app_.config());
    ModList modList(app_.config(), &cache);

    if (format == TEXT)
    {
        cache.refresh(ModCache::LATEST_ONLY);
        modList.refresh();

        maxWidth = 0;
        for (auto mod : modList.mods()) {
        const int width = mod.id().size() + mod.info().name().size() + 3;
        if (width > maxWidth)
            maxWidth = width;
        }

        QTextStream cout(stdout);
        cout.setFieldAlignment(QTextStream::AlignLeft);
        for (auto mod : modList.mods()) {
            writeTextMod(cout, mod);
        }
    }
    else if (format == MODSPEC)
    {
        modList.refresh(ModList::CONTENT_ONLY);

        QTextStream cout(stdout);
        for (auto mod : modList.mods()) {
            writeSpecMod(cout, mod);
        }
    }

    QTimer::singleShot(0, this, &Command::finished);
}

void ModsListCommand::writeSpecMod(QTextStream &cout, const InstalledMod &mod)
{
    cout << mod.id() << ":" << mod.info().name() << Qt::endl;
}

void ModsListCommand::writeTextMod(QTextStream &cout, const InstalledMod &mod)
{
    const ModInfo &info = mod.info();

    cout << qSetFieldWidth(maxWidth) << info.toString() << qSetFieldWidth(0) << "  ";
    cout << qSetFieldWidth(29) << mod.versionString();
    cout << qSetFieldWidth(20);
    if (!mod.hasCacheVersion())
        cout << " (not in cache)";
    else
        cout << ' ';
    cout << qSetFieldWidth(0);

    if (includeHashes)
        cout << ' ' << mod.hash();
    cout << Qt::endl;
}

} // namespace iimodmanager
