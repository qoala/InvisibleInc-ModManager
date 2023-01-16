#include "modelutil.h"
#include "modspecpreviewmodel.h"

#include <QColor>
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
            case PendingChange::RE_ALIAS:
                return QStringLiteral("RE-ALIAS");
            case PendingChange::UPDATE:
                return QStringLiteral("UPDATE");
            default:
                return QString();
        }
    }

    QVariant targetAlias(const PendingChange &pc, Status baseStatus, int role)
    {
        if (pc.type == PendingChange::REMOVE)
            return modelutil::nullData(role, baseStatus);

        if (role == Qt::DisplayRole || role == Qt::EditRole)
            return pc.alias.isEmpty() ? pc.modId : pc.alias;
        else if (role == modelutil::STATUS_ROLE)
        {
            if (pc.alias.isEmpty())
                return modelutil::toVariant(baseStatus | modelutil::UNLABELLED_STATUS);
            else
                return modelutil::toVariant(baseStatus);
        }
        else if (role == Qt::ForegroundRole && pc.alias.isEmpty())
            return QColor(Qt::gray);
        return QVariant();
    }

    QVariant targetAction(const PendingChange &pc, Status baseStatus, int role)
    {
        if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus);

        else if (role == Qt::DisplayRole)
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
            case PendingChange::RE_ALIAS:
                return QStringLiteral("MOVE ALIAS");
            case PendingChange::UPDATE:
                return QStringLiteral("UPDATE");
            }
        else if (role == modelutil::SORT_ROLE)
            // not installed < installed unchanged < update in place < reinstall with new alias < install new < remove existing
            switch (pc.type)
            {
            case PendingChange::NONE:
            case PendingChange::PIN_CURRENT:
            case PendingChange::PIN_LATEST:
                // If we got here, then there's an already-installed version.
                return 1;
            case PendingChange::UPDATE:
                return 2;
            case PendingChange::RE_ALIAS:
                return 3;
            case PendingChange::INSTALL:
                return 4;
            case PendingChange::REMOVE:
                return 5;
            }
        else if (role == Qt::CheckStateRole)
            // We never arrive here unless either there's an already-installed version
            // or we're trying to install one;
            return (pc.type == PendingChange::REMOVE) ? Qt::Unchecked : Qt::Checked;

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
            return modelutil::nullData(role, baseStatus);
        if (role == Qt::ForegroundRole && !pc.isActive())
            return QColor(Qt::gray);
        if (role == Qt::EditRole)
            return cm ? cm->versionIndex(pc.versionId) : -1;

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
            return modelutil::nullData(role, baseStatus);
        if (role == Qt::ForegroundRole && !pc.isActive())
            return QColor(Qt::gray);
        if (role == Qt::EditRole)
            return cm ? cm->versionIndex(pc.versionId) : -1;

        const CachedVersion *cv = cm ? cachedVersion(cm, pc) : nullptr;
        if (!cv && role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus | modelutil::NULL_STATUS);
        return modelutil::versionTimeData(cv, baseStatus, role);
    }
}

static std::optional<ModSpecPreviewModel::PendingChange> pinCurrent(const InstalledMod *im, const ModSpecPreviewModel::PendingChange *previous, const QString &requestedAlias = QString())
{
    using PendingChange = ModSpecPreviewModel::PendingChange;

    if (!im)
        return std::nullopt;

    PendingChange pc(im->id());
    pc.modName = im->info().name();
    pc.alias = previous ? previous->alias : im->alias();
    const CachedVersion *iv = im->cacheVersion();
    if (iv)
        pc.versionId = iv->id();
    else
        pc.versionId.clear();
    pc.versionPin = PendingChange::CURRENT;
    pc.type = pc.alias == im->alias() ? PendingChange::PIN_CURRENT : PendingChange::RE_ALIAS;
    return pc;
}

static std::optional<ModSpecPreviewModel::PendingChange> useLatest(const CachedMod *cm, const InstalledMod *im, const ModSpecPreviewModel::PendingChange *previous)
{
    using PendingChange = ModSpecPreviewModel::PendingChange;

    const CachedVersion *lv = cm ? cm->latestVersion() : nullptr;
    if (!lv || !cm)
        return pinCurrent(im, previous);

    PendingChange pc(cm->id());
    pc.modName = cm->info().name();
    pc.alias = previous ? previous->alias : im ? im->alias() : cm->defaultAlias();
    pc.versionId = lv->id();
    pc.versionPin = PendingChange::LATEST;
    if (!im)
        pc.type = PendingChange::INSTALL;
    else if (pc.alias != im->alias())
        pc.type = PendingChange::RE_ALIAS;
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

    PendingChange base(sm.id());
    base.alias = sm.alias();

    if (sm.versionId().isEmpty())
        return useLatest(cm, im, &base);
    if (sm.versionId() == '-')
    {
        // Keep current, if possible.
        if (im)
            return pinCurrent(im, &base);
        else
            return useLatest(cm, im, &base);
    }
    // A specific version ID was requested.

    const CachedVersion *tv = cm ? cm->version(sm.versionId()) : nullptr;
    if (!tv || !cm)
    {
        emit textOutput(QStringLiteral("! Replacing requested version (%3) with latest for %2 [%1]: Not in cache.")
                .arg(sm.id(), sm.name(), sm.versionId()));
        return useLatest(cm, im, &base);
    }

    PendingChange pc(cm->id());
    pc.modName = cm->info().name();
    pc.alias = sm.alias();
    pc.versionId = tv->id();
    pc.versionPin = PendingChange::PINNED;
    if (!im)
        pc.type = PendingChange::INSTALL;
    else if (pc.alias != im->alias())
        pc.type = PendingChange::RE_ALIAS;
    else if (tv == cm->installedVersion())
        pc.type = PendingChange::PIN_CURRENT;
    else
        pc.type = PendingChange::UPDATE;
    return pc;
}

ModSpecPreviewModel::ModSpecPreviewModel(const ModCache &cache, const ModList &modList, QObject *parent)
    : ModsModel(cache, modList, parent), isLocked_(false), dirty(true), previousAppliableState_(false)
{}

ModSpecPreviewModel::ModSpecPreviewModel(ModCache &cache, const ModList &modList, QObject *parent)
    : ModsModel(cache, modList, parent), isLocked_(false), dirty(true), previousAppliableState_(false)
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

int ModSpecPreviewModel::idColumn() const
{
    return ModSpecPreviewModel::ID;
}

static bool isBase(int column)
{
    switch (column)
    {
    case ModSpecPreviewModel::NAME:
    case ModSpecPreviewModel::ID:
    case ModSpecPreviewModel::DEFAULT_ALIAS:
    case ModSpecPreviewModel::INSTALLED_ALIAS:
    case ModSpecPreviewModel::INSTALLED_VERSION:
    case ModSpecPreviewModel::INSTALLED_VERSION_TIME:
    case ModSpecPreviewModel::LATEST_VERSION:
    case ModSpecPreviewModel::CACHE_UPDATE_TIME:
        return true;
    default:
        return false;
    }
}
static inline bool isBase(const QModelIndex &index)
{
    return isBase(index.column());
}

static int toBaseColumn(int column) {
    switch (column)
    {
    case ModSpecPreviewModel::NAME:
        return ModsModel::NAME;
    case ModSpecPreviewModel::ID:
        return ModsModel::ID;
    case ModSpecPreviewModel::DEFAULT_ALIAS:
        return ModsModel::DEFAULT_ALIAS;
    case ModSpecPreviewModel::INSTALLED_ALIAS:
        return ModsModel::INSTALLED_ALIAS;
    case ModSpecPreviewModel::INSTALLED_VERSION:
        return ModsModel::INSTALLED_VERSION;
    case ModSpecPreviewModel::INSTALLED_VERSION_TIME:
        return ModsModel::INSTALLED_VERSION_TIME;
    case ModSpecPreviewModel::LATEST_VERSION:
        return ModsModel::LATEST_VERSION;
    case ModSpecPreviewModel::CACHE_UPDATE_TIME:
        return ModsModel::CACHE_UPDATE_TIME;
    default:
        return 0;
    }
}

static inline QModelIndex toBaseColumn(const QModelIndex &index) {
    return index.siblingAtColumn(toBaseColumn(index.column()));
}

const ModSpecPreviewModel::PendingChange ModSpecPreviewModel::seekPendingRow(int row, const CachedMod **cm, const InstalledMod **im) const
{
    if (!cm || !im)
        return PendingChange();

    seekRow(row, cm, im);
    if (!(*cm) && !(*im))
        return PendingChange();

    const QString modId = *cm ? (*cm)->id() : *im ? (*im)-> id() : QString();

    if (pendingChanges.contains(modId))
        return pendingChanges.value(modId);
    else
    {
        PendingChange pc(modId);
        pc.alias = *im ? (*im)->alias() : *cm ? (*cm)->defaultAlias() : QString();
        return pc;
    }
}

ModSpecPreviewModel::PendingChange *ModSpecPreviewModel::seekMutablePendingRow(int row, const CachedMod **cm, const InstalledMod **im)
{
    if (!cm || !im)
        return nullptr;

    seekRow(row, cm, im);
    if (!(*cm) && !(*im))
        return nullptr;

    const QString modId = *cm ? (*cm)->id() : *im ? (*im)-> id() : QString();
    PendingChange *pc = &pendingChanges[modId];
    if (pc->modId.isEmpty())
    {
        pc->modId = modId;
        pc->alias = *im ? (*im)->alias() : *cm ? (*cm)->defaultAlias() : QString();
        const auto iv = *im ? (*im)->cacheVersion() : nullptr;
        if (iv)
            pc->versionId = iv->id();
        else
            pc->versionId.clear();
    }
    return pc;
}

QVariant ModSpecPreviewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if ((role != Qt::ForegroundRole && role != Qt::BackgroundRole && role != Qt::ToolTipRole
            && isBase(index)) || role == modelutil::MOD_ID_ROLE)
        return ModsModel::data(toBaseColumn(index), role);
    QModelIndex parent = index.parent();
    if (parent.isValid() && !(role == Qt::DisplayRole && index.column() == ACTION))
        return QVariant(); // Subclass-specific column/role on a child row.

    const CachedMod *cm;
    const InstalledMod *im;
    int modRow = parent.isValid() ? parent.row() : index.row();
    const PendingChange pc = seekPendingRow(modRow, &cm, &im);

    if (parent.isValid())
    {
        int versionRow = index.row();
        const CachedVersion *cv = cm && versionRow < cm->versions().size()
            ? &cm->versions().at(index.row()) : nullptr;
        if (cv && index.column() == ACTION && role == Qt::DisplayRole && cv->installed())
            return QStringLiteral("(installed)");

        return QVariant();
    }

    modelutil::Status baseStatus = modelutil::modStatus(cm, im, role);

    if (role == Qt::BackgroundRole)
    {
        if (pc.hasDupe || pc.isUncachedMove())
            return QColor::fromRgb(255, 0, 0, 64);
        else
            return QVariant();
    }
    else if (role == Qt::ToolTipRole)
    {
        if (pc.hasDupe)
            return QStringLiteral("Can't install multiple mods to the same alias. Change one or both aliases.");
        else if (pc.isUncachedMove())
            return QStringLiteral("Can't preserve an uncached version when changing alias. Import uncached mods via the top menu or select a different version.");
        else
            return QVariant();
    }

    if (!im && pc.isNone())
    {
        // Not installed and not trying to change that.
        if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus | modelutil::NULL_STATUS);
        else if (role == modelutil::SORT_ROLE && index.column() == ACTION)
            // No-status mods are below all marked mods. No-version mods are below event them.
            return (cm && cm->downloaded()) ? 0 : -1;
        else if (role == Qt::CheckStateRole && index.column() == ACTION)
            return Qt::Unchecked;
        else if (role == Qt::ForegroundRole && (!cm || !cm->downloaded()))
            return QColor(Qt::gray);
        else
            return QVariant();
    }

    switch (index.column())
    {
    case DEFAULT_ALIAS:
        if (role == Qt::ForegroundRole && cm && cm->defaultAlias().isEmpty())
            return QColor(Qt::gray);
    case INSTALLED_ALIAS:
        if (role == Qt::ForegroundRole && im && im->alias().isEmpty())
            return QColor(Qt::gray);
    case TARGET_ALIAS:
        return ColumnData::targetAlias(pc, baseStatus, role);
    case ACTION:
        return ColumnData::targetAction(pc, baseStatus, role);
    case TARGET_VERSION:
        return ColumnData::targetVersion(cm, im, pc, baseStatus, role);
    case TARGET_VERSION_TIME:
        return ColumnData::targetTime(cm, pc, baseStatus, role);
    }
    return QVariant();
}

bool ModSpecPreviewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.parent().isValid())
        return false;
    if (index.column() == ACTION && role == Qt::CheckStateRole)
    {
        auto state = value.value<Qt::CheckState>();
        int row = index.row();
        const CachedMod *cm;
        const InstalledMod *im;
        PendingChange *pc = seekMutablePendingRow(row, &cm, &im);
        if (!pc)
            return false;

        if (state == Qt::Checked)
        {
            if (im)
            {
                auto target = pinCurrent(im, pc);
                if (!target)
                    return false;
                *pc = *target;
            }
            else
            {
                // Install new.
                auto target = useLatest(cm, im, pc);
                if (!target)
                    return false;
                *pc = *target;
            }
        }
        else if (state == Qt::Unchecked)
        {
            if (!im)
                pc->type = PendingChange::NONE;
            else if (cm && cm->latestVersion() && cm->latestVersion()->id() != pc->versionId)
            {
                // Pseudo-tri-state. If checked and can update to latest, cycle through update before removing.
                auto target = useLatest(cm, im, pc);
                if (target)
                    *pc = *target;
                else
                    pc->type = PendingChange::REMOVE;
            }
            else
                pc->type = PendingChange::REMOVE;
        }

        setDirty();
        reportSpecChanged(row, true);
        return true;
    }
    else if (index.column() == TARGET_ALIAS && role == Qt::EditRole)
    {
        int row = index.row();
        const CachedMod *cm;
        const InstalledMod *im;
        PendingChange *pc = seekMutablePendingRow(row, &cm, &im);
        if (!pc)
            return false; // Only fails to return a model on error.

        QString newAlias = modelutil::parseIdInput(value.toString().trimmed());
        if (newAlias.isEmpty())
            newAlias = im ? im->alias() : cm ? cm->defaultAlias() : QString();
        else if (newAlias == pc->modId)
            newAlias.clear();

        if (newAlias == pc->alias)
            return false;
        pc->alias = newAlias;

        if (im)
        {
            if (pc->type == PendingChange::REMOVE)
                return false;
            else if (newAlias != im->alias()) // Change alias trumps other chagnes.
                pc->type = PendingChange::RE_ALIAS;
            else if (pc->versionPin == PendingChange::CURRENT) // Return to pinning current version.
                pc->type = PendingChange::PIN_CURRENT;
            else if (cm && im->cacheVersion() != cm->latestVersion()) // Change version.
                pc->type = PendingChange::UPDATE;
            else if (pc->versionPin == PendingChange::LATEST)
                pc->type = PendingChange::PIN_LATEST;
            else
                pc->type = PendingChange::NONE;
        }
        // If we're not supposed to be leaving it uninstalled, this must be a new install.
        else if (pc->type != PendingChange::NONE)
            pc->type = PendingChange::INSTALL;

        setDirty();
        reportSpecChanged(row, true);
        return true;
    }
    else if (role == Qt::EditRole
            && (index.column() == TARGET_VERSION || index.column() == TARGET_VERSION_TIME))
    {
        int row = index.row();
        const CachedMod *cm;
        const InstalledMod *im;
        PendingChange *pc = seekMutablePendingRow(row, &cm, &im);
        if (!pc || !cm)
            return false; // Only fails to return a model on error.

        const QList<CachedVersion> &versions = cm->versions();
        int newIndex = value.toInt();
        if (newIndex < 0 || newIndex >= versions.size())
            return false;
        const CachedVersion &tv = versions.at(newIndex);

        pc->versionId = tv.id();

        if (newIndex == 0)
            pc->versionPin = PendingChange::LATEST;
        else if (tv.installed())
            pc->versionPin = PendingChange::CURRENT;
        else
            pc->versionPin = PendingChange::PINNED;

        if (pc->type == PendingChange::RE_ALIAS)
        {} // No change to type.
        else if (!im)
            pc->type = PendingChange::INSTALL;
        else if (!tv.installed())
            pc->type = PendingChange::UPDATE;
        else if (newIndex == 0)
            pc->type = PendingChange::PIN_LATEST;
        else
            pc->type = PendingChange::PIN_CURRENT;

        setDirty();
        reportSpecChanged(row, true);
        return true;
    }
    return false;
}

QVariant ModSpecPreviewModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case TARGET_ALIAS:
            return QStringLiteral("Target Alias");
        case ACTION:
            return QStringLiteral("Status");
        case TARGET_VERSION:
            return QStringLiteral("Target Version");
        case TARGET_VERSION_TIME:
            return QStringLiteral("Target Version Time");
        }
        break;
    case Qt::InitialSortOrderRole:
        switch (section)
        {
        case TARGET_ALIAS:
            return Qt::AscendingOrder;
        case ACTION:
        case TARGET_VERSION:
        case TARGET_VERSION_TIME:
            return Qt::DescendingOrder;
        }
        break;
    case modelutil::SORT_ROLE:
        switch (section)
        {
        case TARGET_ALIAS:
            return modelutil::MOD_ID_SORT;
        case ACTION:
            return modelutil::ROLE_SORT;
        case TARGET_VERSION:
            return modelutil::VERSION_SORT;
        case TARGET_VERSION_TIME:
            return modelutil::VERSION_TIME_SORT;
        }
        break;
    case modelutil::CANCEL_SORTING_ROLE:
        return QVariant::fromValue<QVector<int>>({ACTION, TARGET_ALIAS, TARGET_VERSION, TARGET_VERSION_TIME});
    }

    return QVariant();
}

Qt::ItemFlags ModSpecPreviewModel::flags(const QModelIndex &index) const
{
    if (index.parent().isValid())
        return QAbstractItemModel::flags(index);
    if (index.column() == ACTION)
    {
        Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        if (!isLocked())
        {
            const CachedMod *cm;
            const InstalledMod *im;
            seekRow(index.row(), &cm, &im);
            if (im || (cm && cm->downloaded()))
                f |= Qt::ItemIsEnabled;
        }
        return f;
    }
    else if (index.column() == TARGET_ALIAS)
    {
        Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEditable;
        if (!isLocked())
        {
            const CachedMod *cm;
            const InstalledMod *im;
            const PendingChange pc = seekPendingRow(index.row(), &cm, &im);
            if (pc.type == PendingChange::INSTALL || (im && pc.type != PendingChange::REMOVE))
                f |= Qt::ItemIsEnabled;
        }
        return f;
    }
    else if (index.column() == TARGET_VERSION || index.column() == TARGET_VERSION_TIME)
    {
        Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEditable;
        if (!isLocked())
        {
            const CachedMod *cm;
            const InstalledMod *im;
            const PendingChange pc = seekPendingRow(index.row(), &cm, &im);
            if (cm && cm->downloaded()
                    && (pc.type == PendingChange::INSTALL
                        || (im && pc.type != PendingChange::REMOVE)))
                f |= Qt::ItemIsEnabled;
        }
        return f;
    }

    return QAbstractItemModel::flags(index);
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

void ModSpecPreviewModel::setLock(bool locked)
{
    isLocked_ = locked;
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
    for (const auto &pc : pendingChanges)
        if (pc.isActive())
            return false;

    return true;
}

bool ModSpecPreviewModel::canApply() const
{
    bool hasChange = false;
    for (const auto &pc : pendingChanges)
    {
        if (pc.isActive())
            hasChange = true;
        if (pc.hasDupe || pc.isUncachedMove())
            return false;
    }

    return hasChange;
}

void ModSpecPreviewModel::checkDuplicates() {
    QSet<QString> removedIds; // Mod IDs, for comparison against current installed mods.
    QHash<QString, PendingChange*> changesByInstalledId; // Installed IDs

    // Check for dupes within the active pending changes.
    for (auto &pc : pendingChanges)
    {
        pc.hasDupe = false;
        if (pc.type == PendingChange::REMOVE || pc.type == PendingChange::RE_ALIAS)
            removedIds << pc.modId;
        if (pc.isActive() && pc.type != PendingChange::REMOVE)
        {
            const QString &id = pc.alias.isEmpty() ? pc.modId : pc.alias;
            if (changesByInstalledId.contains(id))
            {
                pc.hasDupe = true;
                changesByInstalledId[id]->hasDupe = true;
            }
            else
                changesByInstalledId.insert(id, &pc);
        }
    }
    // Check for dupes between active pending changes and currently installed mods.
    for (const auto &im : modList.mods())
    {
        const QString &installedId = im.installedId();
        if (!removedIds.contains(im.id()) && changesByInstalledId.contains(installedId))
        {
            PendingChange *targetChange = changesByInstalledId.value(installedId);
            if (im.id() != targetChange->modId)
            {
                PendingChange &pc = pendingChanges[im.id()];
                if (pc.modId.isEmpty())
                {
                    pc.modId = im.id();
                    pc.alias = im.alias();
                    const auto iv = im.cacheVersion();
                    if (iv)
                        pc.versionId = iv->id();
                    else
                        pc.versionId.clear();
                }
                pc.hasDupe = true;
                targetChange->hasDupe = true;
            }
        }
    }
}

void ModSpecPreviewModel::prepareChanges(QList<SpecMod> *toAddMods, QList<SpecMod> *toUpdateMods, QList<InstalledMod> *toRemoveMods) const
{
    if (!toAddMods || !toUpdateMods || !toRemoveMods || !canApply())
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
        case PendingChange::RE_ALIAS:
        case PendingChange::INSTALL:
            {
                const auto *cm = cache.mod(pc.modId);
                const auto *cv = cm ? cm->version(pc.versionId) : nullptr;
                if (cv)
                    *toAddMods << cv->asSpec().withAlias(pc.alias);
                else
                    emit textOutput(QStringLiteral("! Cannot install %1 '%2': Not in cache.").arg(pc.modId, pc.versionId));
            }
            break;
        case PendingChange::UPDATE:
            {
                const auto *cm = cache.mod(pc.modId);
                const auto *cv = cm ? cm->version(pc.versionId) : nullptr;
                if (cv)
                    *toUpdateMods << cv->asSpec().withAlias(pc.alias);
                else
                    emit textOutput(QStringLiteral("! Cannot update %1 to '%2': Not in cache.").arg(pc.modId, pc.versionId));
            }
            break;
        case PendingChange::REMOVE:
            {
                const auto *im = modList.mod(pc.modId);
                if (im)
                    *toRemoveMods << *im;
                else
                    emit textOutput(QStringLiteral("! Cannot remove %1: No longer installed.").arg(pc.modId, pc.versionId));
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
    if (pc.versionPin == PendingChange::CURRENT)
    {
        const InstalledMod *im = modList.mod(pc.modId);
        const CachedVersion *iv = im ? im->cacheVersion() : nullptr;
        const QString versionId = iv ? iv->id() : QString();
        if (im)
        {
            if (pc.versionId != versionId)
            {
                // Update pinned version.
                pc.versionId = versionId;
                setDirty();
            }
            if (pc.type == PendingChange::RE_ALIAS)
            {
                if (pc.alias == im->alias())
                {
                    pc.type = PendingChange::PIN_CURRENT;
                    setDirty();
                }
            }
            else if (pc.type == PendingChange::PIN_CURRENT)
            {
                if (pc.alias != im->alias())
                {
                    pc.alias = im->alias();
                    setDirty();
                }
            }
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

    // Preserving uncached mods/versions were eliminated as PIN_CURRENT above.
    // Everything else considers an installed version informative, but not critical.
    const InstalledMod *im = modList.mod(pc.modId);
    const CachedVersion *iv = cm->installedVersion();
    if (!iv)
        switch (pc.type)
        {
        default:
            return;
        case PendingChange::PIN_LATEST:
        case PendingChange::INSTALL:
        case PendingChange::RE_ALIAS:
        case PendingChange::UPDATE:
            // Need to install the mod.
            pc.type = PendingChange::INSTALL;
            // Fall through.
        }
    else if (pc.type == PendingChange::REMOVE)
        // Change is clear.
        return;

    // Only PIN_LATEST/INSTALL/RE_ALIAS/UPDATE remain.
    // Check if target versionId is still correct.
    switch (pc.versionPin)
    {
    case PendingChange::CURRENT:
        return; // Eliminated above.
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
    // Re-alias takes precedence over other changes.
    if (im && pc.alias != im->alias())
    {
        pc.type = PendingChange::RE_ALIAS;
        setDirty();
    }
    // Now check if target version is already installed.
    else if (iv && pc.versionId == iv->id())
    {
        if (pc.isActive())
        {
            // Installation/Update complete.
            if (pc.versionPin == PendingChange::LATEST)
                pc.type = PendingChange::PIN_LATEST;
            else if (pc.versionPin == PendingChange::PINNED)
                pc.type = PendingChange::PIN_CURRENT;
            setDirty();
        }
    }
}
void ModSpecPreviewModel::refreshPendingChanges()
{
    for (PendingChange &pc : pendingChanges)
        refreshPendingChange(pc);
}

void ModSpecPreviewModel::reportCacheChanged(const std::function<void ()> &cb, const QString &modId)
{
    emit dataChanged(QModelIndex(), QModelIndex(), {modelutil::CANCEL_SORTING_ROLE});
    ModsModel::reportCacheChanged(cb, modId);
}

void ModSpecPreviewModel::reportAllChanged(const std::function<void ()> &cb, const QString &modId)
{
    if (modId.isEmpty())
        refreshPendingChanges();
    else if (pendingChanges.contains(modId))
        refreshPendingChange(pendingChanges[modId]);

    emit dataChanged(QModelIndex(), QModelIndex(), {modelutil::CANCEL_SORTING_ROLE});
    ModsModel::reportAllChanged(cb, modId);

    checkDuplicates();
    bool emptyState = canApply();
    if (previousAppliableState_ != emptyState)
    {
        previousAppliableState_ = emptyState;
        emit canApplyChanged(emptyState);
    }
}

void ModSpecPreviewModel::reportSpecChanged(int row, bool modifiedByView)
{
    int startRow, endRow;

    if (row == -1)
    {
        // All rows.
        startRow = 0;
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

    checkDuplicates();
    bool emptyState = canApply();
    if (previousAppliableState_ != emptyState)
    {
        previousAppliableState_ = emptyState;
        emit canApplyChanged(emptyState);
    }
}

}  // namespace iimodmanager
