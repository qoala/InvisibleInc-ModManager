#include "modelutil.h"
#include "cacheimportmodel.h"
#include "util.h"

#include <QAbstractItemModel>
#include <QColor>
#include <QRegularExpression>
#include <modcache.h>
#include <moddownloader.h>
#include <modspec.h>
#include <modlist.h>

namespace iimodmanager {

namespace ColumnData {
    typedef modelutil::Status Status;
    typedef CacheImportModel::PendingImport PendingImport;

    QVariant targetAction(const PendingImport &pi, int role)
    {
        if (role == Qt::DisplayRole)
            switch (pi.status)
            {
            case PendingImport::IN_CACHE:
                return QStringLiteral("(already in cache)");
            case PendingImport::NOT_MOD:
                return QStringLiteral("(invalid ID)");
            case PendingImport::PENDING:
                return QStringLiteral("...");
            case PendingImport::NOT_WORKSHOP:
                return QStringLiteral("(local mod)");
            case PendingImport::DOWNLOAD_AVAILABLE:
                return QStringLiteral("(steam mod)");
            case PendingImport::IMPORT_COPY:
                return QStringLiteral("IMPORT LOCAL");
            case PendingImport::IMPORT_DOWNLOAD:
                return QStringLiteral("IMPORT STEAM");
            default:
                return QVariant();
            }
        else if (role == modelutil::SORT_ROLE)
            return QVariant::fromValue<int>(pi.status);
        else if (role == Qt::CheckStateRole)
            return pi.isActive() ? Qt::Checked : Qt::Unchecked;
        return QVariant();
    }
}


CacheImportModel::CacheImportModel(const ModCache &cache, const ModList &modList, ModInfoCall *steamInfoCall, QObject *parent)
    : ModsModel(cache, modList, parent), steamInfoCall(steamInfoCall), steamInfoIdle(true), previousEmptyState_(true)
{
    steamInfoCall->setParent(this);
    connect(steamInfoCall, &ModInfoCall::finished, this, &CacheImportModel::steamInfoFinished);

    // Load pending imports.
    int total = rowCount(QModelIndex());
    for (int i = cache.mods().size(); i < total; ++i)
    {
        const InstalledMod *im;
        auto *pi = seekMutablePendingRow(i, &im);
        // Only care about the side effect of creating the pending entry.
        Q_UNUSED(pi)
    }

    nextInfo();
}

int CacheImportModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

int CacheImportModel::columnMax() const
{
    return COLUMN_MAX;
}

int CacheImportModel::idColumn() const
{
    return CacheImportModel::ID;
}

static bool isBase(int column)
{
    switch (column)
    {
    case CacheImportModel::NAME:
    case CacheImportModel::INSTALLED_VERSION:
        return true;
    default:
        return false;
    }
}
static inline bool isBase(const QModelIndex &index)
{
    return !index.isValid() || isBase(index.column());
}

static int toBaseColumn(int column) {
    switch (column)
    {
    case CacheImportModel::NAME:
        return ModsModel::NAME;
    case CacheImportModel::INSTALLED_VERSION:
        return ModsModel::INSTALLED_VERSION;
    default:
        return 0;
    }
}

static inline QModelIndex toBaseColumn(const QModelIndex &index) {
    return index.siblingAtColumn(toBaseColumn(index.column()));
}

const CacheImportModel::PendingImport CacheImportModel::seekPendingRow(int row, const InstalledMod **im) const
{
    if (!im)
        return PendingImport();

    const CachedMod *cm; // Unused.
    seekRow(row, &cm, im);
    if (!(*im))
        return PendingImport();

    const QString modId = (*im)->id();
    return pendingImport(modId);
}

CacheImportModel::PendingImport *CacheImportModel::seekMutablePendingRow(int row, const InstalledMod **im)
{
    if (!im)
        return nullptr;

    const CachedMod *cm;
    seekRow(row, &cm, im);
    if (!(*im))
        return nullptr;

    const QString modId = (*im)->id();
    PendingImport *pi = &pendingImports[modId];
    if (!pi->isValid())
    {
        pi->installedId = pi->modId = modId;
        if (util::isSteamModId(modId))
            pi->steamId = util::toSteamId(modId);
        pi->updateStatus(cm, nullptr);
    }
    return pi;
}

QVariant CacheImportModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid() || index.row() < cache.mods().size())
        return modelutil::nullData(role); // Ignore child rows and cache rows.
    if (isBase(index) || role == modelutil::MOD_ID_ROLE)
        return ModsModel::data(toBaseColumn(index), role);
    if (role == Qt::BackgroundRole)
    {
        if (index.column() == ACTION || index.column() == ID)
            return QColor::fromRgb(255, 255, 191);
        return QVariant();
    }

    const InstalledMod *im;
    const PendingImport &pi = seekPendingRow(index.row(), &im);

    modelutil::Status baseStatus = modelutil::modStatus(nullptr, im, role);

    if (!im)
    {
        if (role == modelutil::SORT_ROLE && index.column() == ACTION)
            return -1;
        else if (role == Qt::CheckStateRole && index.column() == ACTION)
            return Qt::Unchecked;
        return modelutil::nullData(role, baseStatus);
    }

    switch (index.column())
    {
    case FOLDER:
        if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus);
        else if (role == Qt::DisplayRole)
            return im->installedId();
        break;
    case ACTION:
        if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus);
        return ColumnData::targetAction(pi, role);
    case ID:
        if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus);
        else if (role == Qt::DisplayRole || role == Qt::EditRole)
            return pi.modId;
        break;
    case STEAM_UPDATE_TIME:
        if (pi.steamId.isEmpty() || !infoResults.contains(pi.steamId))
            return modelutil::nullData(role, baseStatus);
        else if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus);
        else if (role == Qt::DisplayRole)
            return infoResults.value(pi.steamId).lastUpdated;
    }
    return QVariant();
}

static bool containsDupe(const QHash<QString, CacheImportModel::PendingImport> &pendingImports, const QString &newId, const QString &existingKey)
{
    for (const auto &pi : pendingImports)
        if (pi.modId == newId && pi.installedId != existingKey)
            return true;
    return false;
}

bool CacheImportModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.parent().isValid() || index.row() < cache.mods().size())
        return false;
    if (index.column() == ACTION && role == Qt::CheckStateRole)
    {
        auto state = value.value<Qt::CheckState>();
        int row = index.row();
        const InstalledMod *im;
        PendingImport *pi = seekMutablePendingRow(row, &im);

        if (!im || !pi)
            return false;

        if (state == Qt::Checked)
        {
            if (!pi->activate())
                return false;
        }
        else if (state == Qt::Unchecked)
        {
            if (!pi->deactivate())
                return false;
        }

        reportImportChanged(row, true);
        return true;
    }
    else if (index.column() == ID && role == Qt::EditRole)
    {
        int row = index.row();
        const InstalledMod *im;
        PendingImport *pi = seekMutablePendingRow(row, &im);

        if (!im || !pi)
            return false;

        QString newId = modelutil::parseIdInput(value.toString().trimmed());
        if (newId.isEmpty())
            newId = pi->installedId;
        if (containsDupe(pendingImports, newId, pi->installedId))
        {
            // Do not allow duplicate target IDs.
            return false;
        }

        bool foundInfo = false;
        SteamModInfo steamInfo;
        pi->modId = newId;
        if (util::isSteamModId(newId))
        {
            pi->steamId = util::toSteamId(newId);
            if (infoResults.contains(pi->steamId))
            {
                foundInfo = true;
                steamInfo = infoResults.value(pi->steamId);
            }
        }
        else
            pi->steamId.clear();

        const CachedMod *cm = cache.mod(newId);
        pi->updateStatus(cm, foundInfo ? &steamInfo : nullptr);

        reportImportChanged(row, true);
        if (pi->status == PendingImport::DOWNLOAD_AVAILABLE)
            pi->status = PendingImport::IMPORT_DOWNLOAD;
        else if (pi->status == PendingImport::PENDING)
            nextInfo();
        return true;
    }
    return false;
}

QVariant CacheImportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();
    if (isBase(section))
        return ModsModel::headerData(toBaseColumn(section), orientation, role);

    switch (role)
    {
    case Qt::DisplayRole:
        switch (section)
        {
        case FOLDER:
            return QStringLiteral("From Alias");
        case ACTION:
            return QStringLiteral("Status");
        case ID:
            return QStringLiteral("ID");
        case STEAM_UPDATE_TIME:
            return QStringLiteral("Latest Steam Update");
        }
        break;
    case Qt::InitialSortOrderRole:
        switch (section)
        {
        case FOLDER:
        case ID:
            return Qt::AscendingOrder;
        case ACTION:
        case STEAM_UPDATE_TIME:
            return Qt::DescendingOrder;
        }
        break;
    case modelutil::SORT_ROLE:
        switch (section)
        {
        case ACTION:
            return modelutil::ROLE_SORT;
        case FOLDER:
        case ID:
            return modelutil::MOD_ID_SORT;
        case STEAM_UPDATE_TIME:
            return modelutil::VERSION_TIME_SORT;
        }
        break;
    case modelutil::CANCEL_SORTING_ROLE:
        return QVariant::fromValue<QVector<int>>({ACTION, ID, STEAM_UPDATE_TIME});
    }

    return QVariant();
}

Qt::ItemFlags CacheImportModel::flags(const QModelIndex &index) const
{
    if (index.parent().isValid() || index.row() < cache.mods().size())
        return QAbstractItemModel::flags(index);
    if (index.column() == ACTION)
    {
        Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        const InstalledMod *im;
        const PendingImport pi = seekPendingRow(index.row(), &im);
        if (pi.isAvailable())
            f |= Qt::ItemIsEnabled;
        return f;
    }
    else if (index.column() == ID)
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index);
}

bool CacheImportModel::isEmpty() const
{
    for (const auto &pi : pendingImports)
        if (pi.isActive())
            return false;

    return true;
}

void CacheImportModel::prepareChanges(QList<SteamModInfo> *toDownloadMods, QList<std::pair<QString, QString>> *toCopyMods) const
{
    if (!toDownloadMods || !toCopyMods)
    {
        emit textOutput(QStringLiteral("! Pointer error in CacheImportModel::prepareChanges"));
        return;
    }

    toDownloadMods->reserve(pendingImports.size());
    toCopyMods->reserve(pendingImports.size());

    for (const auto &pi : pendingImports)
    {
        if (!pi.isActive())
            continue;
        if (cache.contains(pi.modId))
        {
            emit textOutput(QStringLiteral("  Pending import to %1 cancelled: Exists in cache.").arg(pi.modId));
            continue;
        }

        const InstalledMod *im = modList.mod(pi.installedId);
        if (pi.status == PendingImport::IMPORT_DOWNLOAD)
        {
            SteamModInfo steamInfo = infoResults.value(pi.steamId);
            if (steamInfo.valid())
                *toDownloadMods << steamInfo;
            else
            {
                emit textOutput(QStringLiteral("! Pending import to %1 cancelled: Steam info became invalid?").arg(pi.modId));
                continue;
            }
        }
        else if (!im)
            emit textOutput(QStringLiteral("  Pending import for %1 cancelled: No longer in installed.").arg(pi.installedId));

        if (im)
            *toCopyMods << std::pair(im->id(), pi.modId);
    }
}

void CacheImportModel::reportImportChanged(int row, bool modifiedByView)
{
    int startRow, endRow;

    if (row == -1)
    {
        // All uncached rows.
        startRow = cache.mods().size();
        endRow = rowCount() - 1;
    }
    else
        // Update a single mod row.
        startRow = endRow = row;

    if (!modifiedByView)
        emit dataChanged(QModelIndex(), QModelIndex(), {modelutil::CANCEL_SORTING_ROLE});
    emit dataChanged(
            createIndex(startRow, columnMin()),
            createIndex(endRow, columnMax()));

    bool emptyState = isEmpty();
    if (previousEmptyState_ != emptyState)
    {
        previousEmptyState_ = emptyState;
        emit isEmptyChanged(emptyState);
    }
}

void CacheImportModel::steamInfoFinished()
{
    const SteamModInfo &steamInfo = steamInfoCall->result();
    const QString modId = util::fromSteamId(steamInfoCall->workshopId());
    if (!steamInfo.valid())
    {
        emit textOutput(QStringLiteral("  Skipping %1: %2").arg(modId, steamInfoCall->errorDetail()));
        infoResults[steamInfoCall->workshopId()] = SteamModInfo();
    }
    else
    {
        infoResults[steamInfo.id] = steamInfo;
    }

    int row = -1;
    for (auto &pi : pendingImports)
        if (pi.modId == modId)
        {
            row = rowOf(pi.installedId);
            const CachedMod *cm = cache.mod(modId);
            pi.updateStatus(cm, &steamInfo);
            if (pi.status == PendingImport::DOWNLOAD_AVAILABLE)
                pi.status = PendingImport::IMPORT_DOWNLOAD;
            break;
        }

    steamInfoIdle = true;
    nextInfo();

    if (row != -1)
        reportImportChanged(row);
}

void CacheImportModel::nextInfo()
{
    if (!steamInfoIdle)
        return;
    for (const auto &pi : pendingImports)
        if (pi.status == PendingImport::PENDING)
        {
            steamInfoIdle = false;
            steamInfoCall->start(pi.steamId);
            return;
        }
}


void CacheImportModel::PendingImport::updateStatus(const CachedMod *cm, const SteamModInfo *steamInfo)
{
    if (cm)
        status = IN_CACHE;
    else if (steamId.isEmpty())
    {
        if (status != IMPORT_COPY)
            status = NOT_WORKSHOP;
    }
    else if (!steamInfo)
        status = PENDING;
    else if (!steamInfo->valid())
        status = NOT_MOD;
    else if (status != IMPORT_DOWNLOAD)
        status = DOWNLOAD_AVAILABLE;
}

bool CacheImportModel::PendingImport::activate()
{
    switch (status)
    {
        case NOT_WORKSHOP:
            status = IMPORT_COPY;
            return true;
        case DOWNLOAD_AVAILABLE:
            status = IMPORT_DOWNLOAD;
            return true;
        case IMPORT_COPY:
        case IMPORT_DOWNLOAD:
            return true;
        default:
            return false;
    }
}
bool CacheImportModel::PendingImport::deactivate()
{
    switch (status)
    {
        case NOT_WORKSHOP:
        case DOWNLOAD_AVAILABLE:
            return true;
        case IMPORT_COPY:
            status = NOT_WORKSHOP;
            return true;
        case IMPORT_DOWNLOAD:
            status = DOWNLOAD_AVAILABLE;
            return true;
        default:
            return false;
    }
}

} // namespace iimodmanager
