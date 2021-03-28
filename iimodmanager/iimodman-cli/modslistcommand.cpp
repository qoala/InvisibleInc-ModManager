#include "modslistcommand.h"

#include <QPromise>

namespace iimodmanager {

ModsListCommand::ModsListCommand(ModManCliApplication &app)
    : Command(app)
{}

void ModsListCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("list", "Command: List all installed mods");
    parser.addOptions({
                    {"spec", "Format output as a modspec."},
                });
}

QFuture<void> ModsListCommand::executeCommand(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);

    format = parser.isSet("spec") ? MODSPEC : TEXT;

    ModCache cache(app_.config());
    ModList modList(app_.config(), &cache);

    if (format == TEXT)
    {
        cache.refresh();
        modList.refresh();

        maxWidth = 0;
        for (auto mod : modList.mods()) {
        const int width = mod.id().size() + mod.info().name().size();
        if (width > maxWidth)
            maxWidth = width;
        }

        QTextStream cout(stdout);
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

    QPromise<void> promise;
    promise.finish();
    return promise.future();
}

void ModsListCommand::writeSpecMod(QTextStream &cout, const InstalledMod &mod)
{
    cout << mod.id() << ":" << mod.info().name() << Qt::endl;
}

void ModsListCommand::writeTextMod(QTextStream &cout, const InstalledMod &mod)
{
    const ModInfo &info = mod.info();
    const int width = info.id().size() + info.name().size();

    cout << info.toString();
    cout << QString(maxWidth - width, ' ') << "  ::";
    cout << mod.versionString();
    if (!mod.hasCacheVersion())
        cout << " (not in cache)";
    cout << Qt::endl;
}

} // namespace iimodmanager
