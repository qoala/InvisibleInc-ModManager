#include "confirmationprompt.h"
#include "modmancliapplication.h"
#include "modssynccommand.h"

#include <QCommandLineParser>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

ModsSyncCommand::ModsSyncCommand(ModManCliApplication &app)
    : Command(app), isForceSet(false)
{}

void ModsSyncCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("sync", "Command: Sync currently installed mods to the specified mods, installing and uninstalling as necessary.");
    parser.addOptions({
                          {{"s", "spec"}, "Spec file, such as the output of a list --spec command.", "specfile"},
                          {{"f", "force"}, "Remove non-matching mods even if they're not in the cache."},
                      });
}

void ModsSyncCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);

    isForceSet = parser.isSet("force");

    if (parser.isSet("spec"))
    {
        specFileNames.append(parser.values("spec"));
    }
}

void ModsSyncCommand::execute()
{
    cache = new ModCache(app_.config(), this);
    modList = new ModList(app_.config(), cache, this);
    cache->refresh(ModCache::LATEST_ONLY);
    modList->refresh();


    // Read input
    inputSpec.reserve(cache->mods().size());
    bool success = true;
    if (specFileNames.isEmpty())
    {
        for (const auto &im : modList->mods())
            inputSpec.append(im.asSpec().withoutVersion());
    }
    else
    {
        for (auto specFileName : specFileNames)
            success &= readSpecFile(specFileName);
    }
    if (!success)
    {
        QTimer::singleShot(0, this, [this](){ app_.exit(EXIT_FAILURE); });
        return;
    }

    // Match up with cached and installed mods
    qsizetype inputSize = inputSpec.mods().size();
    targetSpec.reserve(inputSize);
    addedMods.reserve(inputSize);
    updatedMods.reserve(inputSize);
    removedMods.reserve(inputSize);
    success = true;
    for (const auto &specMod : inputSpec.mods())
    {
        std::optional<SpecMod> targetSpecMod = makeInstallTarget(specMod);
        if (targetSpecMod)
            targetSpec.append(*targetSpecMod);
        else
            success = false;
    }
    for (const auto &im : modList->mods())
        success &= checkInstalledMod(im);
    if (!success)
    {
        QTimer::singleShot(0, this, [this](){ app_.exit(EXIT_FAILURE); });
        return;
    }

    // Confirm actions
    QTextStream cerr(stderr);
    bool hasAction = false;
    if (!removedMods.empty())
    {
        hasAction = true;
        cerr << "The following mods will be removed:" << Qt::endl << "  ";
        for (const InstalledMod &sm : removedMods)
        cerr << QString("\"%1\" [%2] ").arg(sm.info().name(), sm.id());
        cerr << Qt::endl;
    }
    if (!addedMods.empty())
    {
        hasAction = true;
        cerr << "The following mods will be newly installed:" << Qt::endl << "  ";
        for (const SpecMod &sm : addedMods)
        cerr << QString("\"%1\" [%2 %3] ").arg(sm.name(), sm.id(), sm.versionName());
        cerr << Qt::endl;
    }
    if (!updatedMods.empty())
    {
        hasAction = true;
        cerr << "The following mods will be newly updated:" << Qt::endl << "  ";
        for (const SpecMod &sm : updatedMods)
        cerr << QString("\"%1\" [%2 %3] ").arg(sm.name(), sm.id(), sm.versionName());
        cerr << Qt::endl;
    }

    if (!hasAction)
    {
        cerr << "No mods to update or remove." << Qt::endl;
        QTimer::singleShot(0, this, &Command::finished);
        return;
    }

    prompt = new ConfirmationPrompt(this);
    connect(prompt, &ConfirmationPrompt::yes, this, &ModsSyncCommand::doSync);
    connect(prompt, &ConfirmationPrompt::no, this, [this] {
        QTextStream(stderr) << "Abort." << Qt::endl;
        emit finished();
    });
    prompt->start();
}

void ModsSyncCommand::doSync()
{
    for (const InstalledMod &sm : removedMods)
        if (!removeMod(sm))
            emit finished();
    for (const SpecMod &sm : addedMods)
        if (!installMod(sm))
            emit finished();
    for (const SpecMod &sm : updatedMods)
        if (!installMod(sm))
            emit finished();

    emit finished();
}

bool ModsSyncCommand::readSpecFile(const QString &fileName)
{
    QFile specFile(fileName);
    if (!inputSpec.appendFromFile(specFile, fileName))
    {
        QTextStream cerr(stderr);
        cerr << app_.applicationName() << ": Couldn't read file " << fileName << Qt::endl;
        return false;
    }
    return true;
}

std::optional<SpecMod> ModsSyncCommand::makeInstallTarget(const SpecMod &sm)
{
    const InstalledMod *im = modList->mod(sm.id());
    const CachedMod *cm = cache->mod(sm.id());
    if (!cm && im && sm.versionId().isEmpty())
    {
        // Assume we're just using the existing version.
        return im->asSpec();
    }
    else if (!cm)
    {
        QTextStream(stderr) << "Mod does not exist in cache: " << sm.name() << " [" << sm.id() << ']' << Qt::endl;
        return {};
    }
    if (!cm->downloaded())
    {
        QTextStream(stderr) << "Mod has no downloaded versions: " << cm->info().toString() << Qt::endl;
        return {};
    }

    const CachedVersion *cv = sm.versionId().isEmpty() ? cm->latestVersion() : cm->version(sm.versionId());
    if (!cv)
    {
        QTextStream(stderr) << "Mod version is not in cache: " << cm->info().toString() << ' ' << sm.versionId() << Qt::endl;
        return {};
    }

    SpecMod targetMod = cv->asSpec();

    if (!im)
        addedMods.append(targetMod);
    else if (!cv->installed())
        updatedMods.append(targetMod);

    return targetMod;
}

bool ModsSyncCommand::checkInstalledMod(const InstalledMod &im)
{
    const QString &modId = im.id();
    if (targetSpec.contains(modId))
        return true;

    removedMods.append(im);

    if (!im.hasCacheVersion() && !isForceSet)
    {
        QTextStream(stderr) << "Trying to remove a mod version that isn't saved in the mod cache (-f to override): " << im.info().toString() << ' ' << im.info().version() << Qt::endl;
        return false;
    }

    return true;
}

bool ModsSyncCommand::removeMod(const InstalledMod &im)
{
    if (modList->removeMod(im.id()))
    {
        QTextStream(stderr) << im.info().toString() << " removed" << Qt::endl;
        return true;
    }
    else
    {
        QTextStream(stderr) << "Failed to remove " << im.info().toString() << Qt::endl;
        return false;
    }

}

bool ModsSyncCommand::installMod(const SpecMod &specMod)
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
