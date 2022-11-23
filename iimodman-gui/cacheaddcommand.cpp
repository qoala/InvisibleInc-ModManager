#include "cacheaddcommand.h"
#include "guidownloader.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <modcache.h>
#include <moddownloader.h>
#include <modinfo.h>

namespace iimodmanager {

CacheAddCommand::CacheAddCommand(ModManGuiApplication  &app, QWidget *parent)
    : QObject(parent), app(app)
{}

void CacheAddCommand::execute()
{
    QString input = QStringLiteral("workshop-");
    QStringList modIds;
    while(true)
    {
        bool ok = true;
        input = QInputDialog::getMultiLineText(
                static_cast<QWidget*>(parent()), tr("II Mod Manager: Add Steam Workshop mod"),
                tr("Workshop URLs or mod IDs (\"workshop-123\"), one per line:"),
                input, &ok, Qt::WindowFlags(), Qt::ImhUrlCharactersOnly);
        if (!ok)  // Cancel.
        {
            emit finished();
            deleteLater();
            return;
        }

        QStringList failedIds;
        input = parseIds(input, &modIds, &ok, &failedIds);
        if (ok) break;  // Successfully parsed IDs.

        int result = QMessageBox::warning(static_cast<QWidget*>(parent()), tr("Add Steam Workshop mod"),
                tr("Not a mod ID or URL: %1").arg(failedIds.join(", ")));
        if (result != QMessageBox::Ok)  // Cancel.
        {
            emit finished();
            deleteLater();
            return;
        }
    }

    emit textOutput("\n--");

    app.refreshMods();
    filterNewWorkshopIds(modIds);
    if (workshopIds.isEmpty())
    {
        emit textOutput(QStringLiteral("No new mods requested"));
        emit finished();
        deleteLater();
        return;
    }

    startInfos();
}

// Input handling.

QString CacheAddCommand::parseIds(QString input, QStringList *modIds, bool *ok, QStringList *failedIds)
{
    if (ok) *ok = true;
    bool errors = false;

    QStringList inputItems = input.trimmed().split(QRegularExpression("\\s*\n\\s*"), Qt::SkipEmptyParts);

    modIds->clear();
    modIds->reserve(inputItems.size());
    if (failedIds) failedIds->reserve(inputItems.size());
    for (auto inputItem : inputItems)
    {
        if (util::isSteamModId(inputItem))
        {
            modIds->append(inputItem);
            continue;
        }
        QString parsed = util::parseSteamModUrl(inputItem);
        if (!parsed.isEmpty())
        {
            modIds->append(parsed);
            continue;
        }
        errors = true;
        modIds->append(inputItem);  // Converting the parsed IDs back into a new input.
        if (failedIds) failedIds->append(inputItem);
    }

    if (errors)
    {
        if (ok) *ok = false;
        QString newInput = modIds->join("\n");
        modIds->clear();
        return newInput;
    }
    else
    {
        return QString();
    }
}

void CacheAddCommand::filterNewWorkshopIds(QStringList modIds)
{
    workshopIds.clear();
    workshopIds.reserve(modIds.size());
    bool skipped = false;
    for (QString modId : modIds)
    {
        const CachedMod *cm = app.cache().mod(modId);
        if (cm)
        {
            emit textOutput(QStringLiteral("Skipping already-downloaded \"%2\" [%1].").arg(modId, cm->info().name()));
            skipped = true;
            continue;
        }
        const ModInfo cachedInfo = ModInfo(modId);
        if (!cachedInfo.isSteam())
        {
            // Safety check. This should've been caught by parseIds.
            emit textOutput(QStringLiteral("Skipping malformed workshop ID [%1].").arg(modId));
            continue;
        }
        workshopIds.append(cachedInfo.steamId());
    }

    if (skipped)
        emit textOutput(QStringLiteral("Use \"Cache > Update Cache\" to download new versions.\n"));
}

// Download handling.

void CacheAddCommand::startInfos()
{
    steamInfoCall = app.modDownloader().modInfoCall();
    steamInfoCall->setParent(this);
    connect(steamInfoCall, &ModInfoCall::finished, this, &CacheAddCommand::steamInfoFinished);

    steamInfos.clear();
    steamInfos.reserve(workshopIds.size());
    emit beginProgress(workshopIds.size());

    loopIndex = 0;
    steamInfoCall->start(workshopIds.first());
}

void CacheAddCommand::nextSteamInfo()
{
    if (++loopIndex < workshopIds.size())
    {
        // Next steamInfo
        emit updateProgress(loopIndex);
        steamInfoCall->start(workshopIds.at(loopIndex));
    }
    else if (steamInfos.empty())
    {
        // No downloads needed.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;

        emit updateProgress(loopIndex);
        emit textOutput("No downloadable mods.");
        emit finished();
        deleteLater();
    }
    else
    {
        // Begin downloading.
        steamInfoCall->deleteLater();
        steamInfoCall = nullptr;

        emit beginProgress(steamInfos.size());
        emit textOutput(QString("Downloading %1 mods...").arg(steamInfos.size()));
        startDownloads();
    }
}

void CacheAddCommand::steamInfoFinished()
{
    const SteamModInfo &steamInfo = steamInfoCall->result();
    if (!steamInfo.valid())
    {
        emit textOutput(QString("  Skipping workshop-%1: %2").arg(steamInfoCall->workshopId(), steamInfoCall->errorDetail()));
        nextSteamInfo();
        return;
    }

    const QString modId = steamInfo.modId();
    const CachedMod *cachedMod = app.cache().contains(modId) ? app.cache().mod(modId) : app.cache().addUnloaded(steamInfo);
    if (!cachedMod)
    {
        emit textOutput(QString("  Skipping %1: cache error.").arg(modId));
    }
    else
    {
        steamInfos.append(steamInfo);
    }
    nextSteamInfo();
}

void CacheAddCommand::startDownloads()
{
    auto *downloader = new GuiModDownloader(app, steamInfos, this);
    connect(downloader, &GuiModDownloader::finished, this, &CacheAddCommand::modDownloadFinished);
    connect(downloader, &GuiModDownloader::textOutput, this, &CacheAddCommand::textOutput);
    connect(downloader, &GuiModDownloader::beginProgress, this, &CacheAddCommand::beginProgress);
    connect(downloader, &GuiModDownloader::updateProgress, this, &CacheAddCommand::updateProgress);

    downloader->execute();
}

void CacheAddCommand::modDownloadFinished()
{
    emit textOutput("Finished downloading new mods.");

    app.cache().saveMetadata();
    emit finished();
    deleteLater();
}

} // namespace iimodmanager
