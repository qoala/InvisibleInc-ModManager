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

    QVariant targetAction(const CachedMod *cm, const InstalledMod *im, const PendingChange &pc, Status baseStatus, int role)
    {
        if (role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus);
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (pc.type)
        {
            case PendingChange::NONE:
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
        QString version = versionString(cm, im, pc);
        return modelutil::versionData(version, baseStatus, role);
    }

    const CachedVersion *cachedVersion(const CachedMod *cm, const PendingChange &pc)
    {
        if (pc.isNone())
            return cm->installedVersion();
        else
            return cm->version(pc.versionId);
    }

    QVariant targetTime(const CachedMod *cm, const InstalledMod *im, const PendingChange &pc, Status baseStatus, int role)
    {
        const CachedVersion *cv = cm ? cachedVersion(cm, pc) : nullptr;
        if (!cv && role == modelutil::STATUS_ROLE)
            return modelutil::toVariant(baseStatus | modelutil::NULL_STATUS);
        return modelutil::versionTimeData(cv, baseStatus, role);
    }
}

ModSpecPreviewModel::ModSpecPreviewModel(const ModCache &cache, const ModList &modList, QObject *parent)
    : ModsModel(cache, modList, parent)
{}

int ModSpecPreviewModel::columnCount(const QModelIndex &parent) const
{
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
    const PendingChange pc = pendingChanges.contains(modId) ? pendingChanges.value(modId) : PendingChange();

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
        return ColumnData::targetAction(cm, im, pc, baseStatus, role);
    case TARGET_VERSION:
        return ColumnData::targetVersion(cm, im, pc, baseStatus, role);
    case TARGET_VERSION_TIME:
        return ColumnData::targetTime(cm, im, pc, baseStatus, role);
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

}  // namespace iimodmanager
