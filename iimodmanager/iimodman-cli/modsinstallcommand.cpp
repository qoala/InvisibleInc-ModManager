#include "confirmationprompt.h"
#include "modmancliapplication.h"
#include "modsinstallcommand.h"

#include <QCommandLineParser>
#include <QTimer>
#include <memory>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

ModsInstallCommand::ModsInstallCommand(ModManCliApplication &app)
    : Command(app)
{}

void ModsInstallCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("install", "Command: Install the specified mods from cache. Does not remove other mods.");
    parser.addOptions({
                          {{"m", "mod-id"}, "Install mods by ID (e.g. 'workshop-2151835746'), may be repeated.", "id"},
                      });
}

void ModsInstallCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);

    if (parser.isSet("mod-id"))
        modIds.append(parser.values("mod-id"));
}

void ModsInstallCommand::execute()
{
    cache = new ModCache(app_.config(), this);
    modList = new ModList(app_.config(), cache, this);
    cache->refresh(ModCache::LATEST_ONLY);
    modList->refresh();

    specMods.clear();
    specMods.reserve(modIds.size());
    for (const QString &modId : modIds)
    {
        const auto spec = specForLatest(modId);
        if (spec)
            specMods.append(*spec);
    }

    QTextStream cerr(stderr);
    if (specMods.empty())
    {
        cerr << "No mods to install." << Qt::endl;
        QTimer::singleShot(0, this, &Command::finished);
        return;
    }

    cerr << "The following mods will be installed:" << Qt::endl << "  ";
    for (const SpecMod &sm : specMods)
        cerr << QString("\"%1\" [%2 %3] ").arg(sm.name(), sm.id(), sm.versionName());
    cerr << Qt::endl;

    prompt = new ConfirmationPrompt(this);
    connect(prompt, &ConfirmationPrompt::yes, this, &ModsInstallCommand::doInstalls);
    connect(prompt, &ConfirmationPrompt::no, this, [this] {
        QTextStream(stderr) << "Abort." << Qt::endl;
        emit finished();
    });
    prompt->start();
}

void ModsInstallCommand::doInstalls()
{
    for (const SpecMod &sm : specMods)
        installMod(sm);

    emit finished();
}

std::optional<const SpecMod> ModsInstallCommand::specForLatest(const QString &modId)
{
    const CachedMod *cm = cache->mod(modId);
    if (!cm)
    {
        QTextStream(stderr) << "Mod is not in cache: " << modId << Qt::endl;
        return {};
    }
    if (!cm->downloaded())
    {
        QTextStream(stderr) << "Mod has no downloaded versions in cache: " << cm->info().toString() << Qt::endl;
        return {};
    }
    const CachedVersion *latest = cm->latestVersion();
    const InstalledMod *im = modList->mod(modId);
    if (im)
    {
        const CachedVersion *iv = im->cacheVersion();
        if (iv->id() == latest->id())
        {
            QTextStream(stderr) << "Mod is already up to date: " << cm->info().toString() << Qt::endl;
            return {};
        }
    }
    return latest->asSpec();
}

bool ModsInstallCommand::installMod(const SpecMod &specMod)
{
    const InstalledMod *installed = modList->installMod(specMod);
    if (installed)
    {
        QTextStream(stderr) << installed->info().toString() << " installed " << installed->info().version() << Qt::endl;
    }
    else
    {
        const CachedMod *cm = cache->mod(specMod.id());
        QTextStream(stderr) << "Failed to install " << cm->info().toString() << Qt::endl;
    }
    return installed;
}

} // namespace iimodmanager
