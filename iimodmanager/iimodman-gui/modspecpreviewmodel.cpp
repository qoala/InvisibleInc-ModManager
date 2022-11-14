#include "modelutil.h"
#include "modsmodel.h"
#include "modspecpreviewmodel.h"

#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

namespace ColumnData {
    typedef modelutil::Status Status;
    typedef ModSpecPreviewModel::PendingChange PendingChange;

    //! For info formatting, not normal column display.
    QString changeType(PendingChange::ChangeType type)
    {
        switch (type)
        {
            case PendingChange::PIN_CURRENT:
            case PendingChange::PIN_LATEST:
                return QStringLiteral("PINNED");
            case PendingChange::INSTALL:
                return QStringLiteral("INSTALL");
            case PendingChange::REMOVE:
                return QStringLiteral("REMOVE");
            case PendingChange::UPDATE:
                return QStringLiteral("UPDATE");
            default:
                return QString();
        }
    }

    QVariant targetAction(const PendingChange &pc, Status baseStatus, int role)
    {
        if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus);
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (pc.type)
        {
            case PendingChange::NONE:
            case PendingChange::PIN_CURRENT:
            case PendingChange::PIN_LATEST:
                // If we got here, then there's an already-installed version.
                return QStringLiteral("(installed)");
            case PendingChange::INSTALL:
                return QStringLiteral("INSTALL");
            case PendingChange::REMOVE:
                return QStringLiteral("REMOVE");
            case PendingChange::UPDATE:
                return QStringLiteral("UPDATE");
        }
        return QVariant();
    }

    QString versionString(const CachedMod *cm, const InstalledMod *im, const PendingChange &pc)
    {
        if (pc.isNone())
            // Should always have im here, but be safe.
            return im ? im->info().version() : QString();

        const CachedVersion *cv = cm ? cm->version(pc.versionId) : nullptr;
        if (cv)
            return cv->info().version();
        else
            return QString();
    }

    QVariant targetVersion(const CachedMod *cm, const InstalledMod *im, const PendingChange &pc, Status baseStatus, int role)
    {
        if (pc.type == PendingChange::REMOVE)
        {
            if (role == modelutil::STATUS_ROLE)
                return modelutil::toVariant(baseStatus | modelutil::NULL_STATUS);
            else
                return QVariant();
        }

        QString version = versionString(cm, im, pc);
        return modelutil::versionData(version, baseStatus, role);
    }

    const CachedVersion *cachedVersion(const CachedMod *cm, const PendingChange &pc)
    {
        if (!cm)
            return nullptr;
        if (pc.isNone())
            return cm->installedVersion();
        else
            return cm->version(pc.versionId);
    }

    QVariant targetTime(const CachedMod *cm, const PendingChange &pc, Status baseStatus, int role)
    {
        if (pc.type == PendingChange::REMOVE)
        {
            if (role == modelutil::STATUS_ROLE)
                return modelutil::toVariant(baseStatus | modelutil::NULL_STATUS);
            else
                return QVariant();
        }

        const CachedVersion *cv = cm ? cachedVersion(cm, pc) : nullptr;
        if (!cv && role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus | modelutil::NULL_STATUS);
        return modelutil::versionTimeData(cv, baseStatus, role);
    }
}

ModSpecPreviewModel::ModSpecPreviewModel(const ModCache &cache, const ModList &modList, QObject *parent)
    : ModsModel(cache, modList, parent), dirty(true)
{}

int ModSpecPreviewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

int ModSpecPreviewModel::columnMax() const
{
    return COLUMN_MAX;
}

bool isBase(int column)
{
    return (column >= ModSpecPreviewModel::BASE_COLUMN_MIN
            && column <= ModSpecPreviewModel::BASE_COLUMN_MAX);
}
bool isBase(const QModelIndex &index)
{
    return (!index.isValid()
            || (!index.parent().isValid() && isBase(index.column())));
}

inline int toBaseColumn(int column) { return column; }
inline QModelIndex toBaseColumn(const QModelIndex &index) { return index; }

QVariant ModSpecPreviewModel::data(const QModelIndex &index, int role) const
{
    if (isBase(index))
        return ModsModel::data(toBaseColumn(index), role);

    const CachedMod *cm;
    const InstalledMod *im;
    seekRow(index.row(), &cm, &im);
    const QString modId = cm ? cm->id() : im ? im-> id() : QString();
    const PendingChange pc = pendingChange(modId);

    modelutil::Status baseStatus = modelutil::modStatus(cm, im, role);

    if (!im && pc.isNone())
    {
        // Not installed and not trying to change that.
        if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus | modelutil::NULL_STATUS);
        else
            return QVariant();
    }

    switch (index.column())
    {
    case ACTION:
        return ColumnData::targetAction(pc, baseStatus, role);
    case TARGET_VERSION:
        return ColumnData::targetVersion(cm, im, pc, baseStatus, role);
    case TARGET_VERSION_TIME:
        return ColumnData::targetTime(cm, pc, baseStatus, role);
    }
    return QVariant();
}

QVariant ModSpecPreviewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();
    if (isBase(section))
        return ModsModel::headerData(toBaseColumn(section), orientation, role);

    if (role == Qt::DisplayRole)
        switch (section)
        {
        case ACTION:
            return QStringLiteral("Status");
        case TARGET_VERSION:
            return QStringLiteral("Target Version");
        case TARGET_VERSION_TIME:
            return QStringLiteral("Target Version Time");
        }
    else if (role == Qt::InitialSortOrderRole)
        switch (section)
        {
        case ACTION:
            return Qt::AscendingOrder;
        case TARGET_VERSION:
        case TARGET_VERSION_TIME:
            return Qt::DescendingOrder;
        }
    else if (role == modelutil::SORT_ROLE)
        switch (section)
        {
        case ACTION:
            return modelutil::NORMAL_SORT;
        case TARGET_VERSION:
            return modelutil::VERSION_SORT;
        case TARGET_VERSION_TIME:
            return modelutil::VERSION_TIME_SORT;
        }
    else if (role == Qt::BackgroundRole)
        // TODO
        return QVariant();

    return QVariant();
}

std::optional<ModSpecPreviewModel::PendingChange> pinCurrent(const InstalledMod *im)
{
    using PendingChange = ModSpecPreviewModel::PendingChange;

    if (!im)
        return std::nullopt;

    PendingChange pc(im->id());
    pc.modName = im->info().name();
    if (const CachedVersion *iv = im->cacheVersion())
        pc.versionId = iv->id();
    pc.versionPin = PendingChange::CURRENT;
    pc.type = PendingChange::PIN_CURRENT;
    return pc;
}

std::optional<ModSpecPreviewModel::PendingChange> useLatest(const CachedMod *cm, const InstalledMod *im)
{
    using PendingChange = ModSpecPreviewModel::PendingChange;

    const CachedVersion *lv = cm ? cm->latestVersion() : nullptr;
    if (!lv || !cm)
        return pinCurrent(im);

    PendingChange pc(cm->id());
    pc.modName = cm->info().name();
    pc.versionId = lv->id();
    pc.versionPin = PendingChange::LATEST;
    if (!im)
        pc.type = PendingChange::INSTALL;
    else if (lv == cm->installedVersion())
        pc.type = PendingChange::PIN_LATEST;
    else
        pc.type = PendingChange::UPDATE;
    return pc;
}

std::optional<ModSpecPreviewModel::PendingChange> ModSpecPreviewModel::toPendingChange(const SpecMod &sm) const
{
    if (sm.id().isEmpty())
        return std::nullopt;

    const CachedMod *cm = cache.mod(sm.id());
    const InstalledMod *im = modList.mod(sm.id());
    if (!cm && !im)
    {
        emit textOutput(QStringLiteral("! Skipping %2 [%1]: Not in cache or installed.")
                .arg(sm.id(), sm.name()));
        return std::nullopt;
    }

    if (sm.versionId().isEmpty())
        return useLatest(cm, im);
    if (sm.versionId() == '-')
    {
        // Keep current, if possible.
        if (im)
            return pinCurrent(im);
        else
            return useLatest(cm, im);
    }

    const CachedVersion *tv = cm ? cm->version(sm.versionId()) : nullptr;
    if (!tv || !cm)
    {
        emit textOutput(QStringLiteral("! Replacing requested version (%3) with latest for %2 [%1]: Not in cache.")
                .arg(sm.id(), sm.name(), sm.versionId()));
        return useLatest(cm, im);
    }

    PendingChange pc(cm->id());
    pc.modName = cm->info().name();
    pc.versionId = tv->id();
    pc.versionPin = PendingChange::PINNED;
    if (!im)
        pc.type = PendingChange::INSTALL;
    else if ((tv = cm->installedVersion()))
        pc.type = PendingChange::PIN_CURRENT;
    else
        pc.type = PendingChange::UPDATE;
    return pc;
}

void ModSpecPreviewModel::setModSpec(const QList<SpecMod> &specMods)
{
    pendingChanges.clear();

    // Create a pending change for each mod.
    for (const auto &sm : specMods)
        if (auto pc = toPendingChange(sm))
            pendingChanges.insert(pc->modId, *pc);

    // Mark removal of each installed mod that's not in the spec.
    for (const auto &im : modList.mods())
        if (!pendingChanges.contains(im.id()))
        {
            PendingChange &pc = *pendingChanges.insert(im.id(), PendingChange(im.id()));
            pc.modName = im.info().name();
            pc.type = PendingChange::REMOVE;
        }

    setDirty();
    reportSpecChanged();
}

void ModSpecPreviewModel::insertModSpec(const QList<SpecMod> &specMods)
{
    // Create a pending change for each mod.
    for (const auto &sm : specMods)
        if (auto pc = toPendingChange(sm))
            pendingChanges.insert(pc->modId, *pc);
        else
            pendingChanges.remove(sm.id());

    setDirty();
    reportSpecChanged();
}

QList<SpecMod> ModSpecPreviewModel::modSpec() const
{
    if (dirty)
        generateModSpec();
    return modSpec_;
}

QList<SpecMod> ModSpecPreviewModel::versionedModSpec() const
{
    if (dirty)
        generateModSpec();
    return versionedModSpec_;
}

bool ModSpecPreviewModel::isEmpty() const
{
    if (pendingChanges.isEmpty())
        return true;

    for (const auto &change : pendingChanges)
        if (change.type >= PendingChange::ACTIVE_CHANGE_MIN)
            return false;

    return true;
}

void ModSpecPreviewModel::prepareChanges(QList<SpecMod> *toAddMods, QList<SpecMod> *toUpdateMods, QList<InstalledMod> *toRemoveMods) const
{
    if (!toAddMods || !toUpdateMods || !toRemoveMods)
    {
        emit textOutput(QStringLiteral("! Pointer error in ModSpecPreviewModel::prepareChanges"));
        return;
    }

    toAddMods->reserve(pendingChanges.size());
    toUpdateMods->reserve(pendingChanges.size());
    toRemoveMods->reserve(pendingChanges.size());

    for (const auto &pc : pendingChanges)
        switch (pc.type)
        {
        case PendingChange::INSTALL:
            {
                const auto *cm = cache.mod(pc.modId);
                const auto *cv = cm ? cm->version(pc.versionId) : nullptr;
                if (cv)
                    *toAddMods << cv->asSpec();
            }
            break;
        case PendingChange::UPDATE:
            {
                const auto *cm = cache.mod(pc.modId);
                const auto *cv = cm ? cm->version(pc.versionId) : nullptr;
                if (cv)
                    *toUpdateMods << cv->asSpec();
            }
            break;
        case PendingChange::REMOVE:
            {
                const auto *im = modList.mod(pc.modId);
                if (im)
                    *toRemoveMods << *im;
            }
            break;
        default:
            break;
        }
}

void ModSpecPreviewModel::revert()
{
    // Actually empty, not just "no changes" empty.
    if (!pendingChanges.isEmpty())
    {
        pendingChanges.clear();
        setDirty();
        reportSpecChanged();
    }
}

void ModSpecPreviewModel::generateModSpec() const
{
    // TODO
}

void ModSpecPreviewModel::refreshPendingChange(PendingChange &pc)
{
    if (pc.isNone())
        return;

    // These cases have their own handling.
    if (pc.type == PendingChange::REMOVE)
    {
        if (!modList.contains(pc.modId))
            // Already done.
            pc.type = PendingChange::NONE;
        return;
    }
    if (pc.type == PendingChange::PIN_CURRENT)
    {
        const InstalledMod *im = modList.mod(pc.modId);
        const CachedVersion *iv = im ? im->cacheVersion() : nullptr;
        const QString versionId = iv ? iv->id() : QString();
        if (im && pc.versionId == versionId)
        {}    // Already done.
        else if (im)
        {
            // Update pinned version.
            pc.versionId = versionId;
            setDirty();
        }
        else
        {
            // It's gone now.
            pc.type = PendingChange::NONE;
            setDirty();
        }
        return;
    }

    const CachedMod *cm = cache.mod(pc.modId);
    const CachedVersion *lv = cm ? cm->latestVersion() : nullptr;
    if (!lv || !cm)
    {
        // Can't apply this change anymore.
        emit textOutput(QStringLiteral("! Dropping pending change (%3) for %2 [%1]: No longer in cache.")
                .arg(pc.modName, pc.modId, ColumnData::changeType(pc.type)));
        pc.type = PendingChange::NONE;
        setDirty();
        return;
    }

    // Preserving Installed-only Uncached mods should be eliminated as PIN_CURRENT above.
    const CachedVersion *iv = cm->installedVersion();
    if (!iv)
        switch (pc.type)
        {
        default:
            return;
        case PendingChange::PIN_LATEST:
        case PendingChange::INSTALL:
        case PendingChange::UPDATE:
            // Need to install the mod.
            pc.type = PendingChange::INSTALL;
            // Fall through.
        }
    else if (pc.type == PendingChange::REMOVE)
        // Change is clear.
        return;

    // Only PIN_CURRENT/INSTALL/UPDATE remain.
    // Check if target versionId is still correct.
    switch (pc.versionPin)
    {
    case PendingChange::CURRENT:
        if (iv && pc.versionId != iv->id())
        {
            // Current has changed. Abandon attempts to keep it pinned.
            pc.type = PendingChange::NONE;
            pc.versionId = QString();
            setDirty();
        }
        // Target version matches (or abandoned) installed version. Done here.
        return;
    case PendingChange::LATEST:
        if (pc.versionId != lv->id())
        {
            pc.versionId = lv->id();
            setDirty();
        }
        break;
    case PendingChange::PINNED:
        const CachedVersion *tv = ColumnData::cachedVersion(cm, pc);
        // If the pinned version exists, everything is good. If not...
        if (!tv)
        {
            emit textOutput(QStringLiteral("! Replacing requested version (%3) with latest for %2 [%1]: No longer in cache.")
                    .arg(pc.modId, pc.modName, pc.versionId));
            pc.versionId = lv->id();
            pc.versionPin = PendingChange::LATEST;
            setDirty();
        }
    }
    // Now check if target version is already installed.
    if (iv && pc.versionId == iv->id())
    {
        if (pc.type >= PendingChange::ACTIVE_CHANGE_MIN)
        {
            // Installation/Update complete.
            if (pc.versionPin == PendingChange::LATEST)
                pc.type = PendingChange::PIN_LATEST;
            if (pc.versionPin == PendingChange::PINNED)
                pc.type = PendingChange::PIN_CURRENT;
        }
    }
}
void ModSpecPreviewModel::refreshPendingChanges()
{
    for (PendingChange &pc : pendingChanges)
        refreshPendingChange(pc);
}

void ModSpecPreviewModel::reportAllChanged(const std::function<void ()> &cb, const QString &modId)
{
    if (modId.isEmpty())
        refreshPendingChanges();
    else if (pendingChanges.contains(modId))
        refreshPendingChange(pendingChanges[modId]);
    ModsModel::reportAllChanged(cb, modId);
}

void ModSpecPreviewModel::reportSpecChanged(const QString &modId)
{
    int startRow, endRow;

    if (modId.isEmpty())
    {
        // All rows.
        startRow = 0;
        endRow = rowCount() - 1;
    }
    else if ((startRow = rowOf(modId)) == -1)
        // Mod is not present.
        return;
    else
        // Update a single mod row.
        endRow = startRow;

    emit dataChanged(
            createIndex(startRow, NEW_COLUMN_MIN),
            createIndex(endRow, NEW_COLUMN_MAX));
}

}  // namespace iimodmanager
