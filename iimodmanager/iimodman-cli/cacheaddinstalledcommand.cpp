#include "cacheaddinstalledcommand.h"
#include "confirmationprompt.h"
#include "updatemodsimpl.h"

#include <QCommandLineParser>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

CacheAddInstalledCommand::CacheAddInstalledCommand(ModManCliApplication &app)
    : Command(app)
{}

void CacheAddInstalledCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("add-installed", "Command: Add all currently installed mods to the cache.");
}

void CacheAddInstalledCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(parser);
    Q_UNUSED(args);
}

void CacheAddInstalledCommand::execute()
{
    cache = new ModCache(app_.config(), this);
    modList = new ModList(app_.config(), cache, this);
    updateImpl = new UpdateModsImpl(app_, cache, nullptr, this);
    updateImpl->setVerb(UpdateModsImpl::VERB_DOWNLOAD);
    updateImpl->setMissingCacheAction(UpdateModsImpl::CACHE_ADD);
    updateImpl->setVerbose(true);

    cache->refresh(ModCache::LATEST_ONLY);
    modList->refresh(ModList::FULL);

    // all mods to be added
    modIds.clear();
    modIds.reserve(modList->mods().size());
    // mods that will be found on steam
    QStringList steamModIds;
    steamModIds.reserve(modList->mods().size());
    for (const InstalledMod &m : modList->mods())
    {
        modIds.append(m.id());
        if (m.info().isSteam() && !m.hasCacheVersion())
            steamModIds.append(m.id());
    }

    updateImpl->start(steamModIds);
    connect(updateImpl, &UpdateModsImpl::finished, this, &CacheAddInstalledCommand::updateFinished);
}

void CacheAddInstalledCommand::updateFinished()
{
    if (!updateImpl->success())
    {
        // Abort the operation.
        emit finished();
        return;
    }

    // Find installed versions in the updated cache.
    modList->refresh(ModList::FULL);

    uncachedMods.clear();
    uncachedMods.reserve(modList->mods().size());
    for (const InstalledMod &m : modList->mods())
    {
        if (!m.hasCacheVersion())
            uncachedMods.append(m);
    }

    QTextStream cerr(stderr);
    if (uncachedMods.empty())
    {
        cerr << "All installed mod versions in cache." << Qt::endl;
        emit finished();
        return;
    }

    cerr << "The currently installed version will be copied to cache for the following mods:" << Qt::endl << "  ";
    for (const InstalledMod &m : uncachedMods)
        cerr << QString("\"%1\" [%2] ").arg(m.info().name(), m.id());
    cerr << Qt::endl;

    prompt = new ConfirmationPrompt(this);
    connect(prompt, &ConfirmationPrompt::yes, this, &CacheAddInstalledCommand::startCopy);
    connect(prompt, &ConfirmationPrompt::no, this, [this] {
        QTextStream(stderr) << "Abort." << Qt::endl;
        emit finished();
    });
    prompt->start();
}

void CacheAddInstalledCommand::startCopy()
{
    for (const InstalledMod &m : uncachedMods)
        copyMod(m);

    emit finished();
}

bool CacheAddInstalledCommand::copyMod(const InstalledMod &installed)
{
    QString versionId;
    const CachedMod *m = cache->mod(installed.id());
    if (installed.info().isSteam() && !m)
    {
        versionId = QStringLiteral("000-original");
    }
    else if (installed.info().isSteam())
    {
        int i;
        for (i = 0; i < 100; ++i)
        {
            versionId = QStringLiteral("%1-original").arg(i, 3, 10, QLatin1Char('0'));
            if (!m->containsVersion(versionId))
                break;
        }
        if (i == 100)
        {
            QTextStream(stderr) << "Exceeded IDs for original versions of " << installed.info().toString() << Qt::endl;
            return false;
        }
    }
    else
    {
        versionId = QStringLiteral("dev");
    }

    const CachedVersion *v = cache->addModVersion(installed.id(), versionId, installed.path());
    QTextStream(stderr) << v->info().toString() << " copied " << v->toString(FORMAT_FULL) << " from installed" << Qt::endl;
    return true;
}

} // namespace iimodmanager
