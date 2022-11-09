#include "modcachemodel.h"

#include <QDateTime>
#include <modinfo.h>

namespace iimodmanager {

namespace Columns {
    QVariant modName(const CachedMod &mod, int role)
    {
        if (role == Qt::DisplayRole)
            return mod.info().name();
        else
            return QVariant();
    }

    QVariant modId(const CachedMod &mod, int role)
    {
        if (role == Qt::DisplayRole)
            return mod.info().id();
        else
            return QVariant();
    }

    QVariant modLatestVersion(const CachedMod &mod, int role)
    {
        const CachedVersion *cv = mod.latestVersion();
        if (!cv)
        {
            if (role == Qt::DisplayRole)
                return QStringLiteral("N/A");
            else if (role == Qt::ToolTipRole)
                return QStringLiteral("(needs download)");
            else
                return QVariant();
        }

        const std::optional<QString> version = cv->version();
        if (!version)
        {
            if (role == Qt::DisplayRole)
                return QStringLiteral("-");
            else if (role == Qt::ToolTipRole)
                return QStringLiteral("(unlabeled)");
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return *version;
        else
            return QVariant();
    }

    QVariant modLatestUpdateTime(const CachedMod &mod, int role)
    {
        const CachedVersion *cv = mod.latestVersion();
        if (!cv)
            return QVariant();

        const std::optional<QDateTime> timestamp = cv->timestamp();
        if (!timestamp)
        {
            if (role == Qt::DisplayRole)
                return cv->id();
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return *timestamp;
        else
            return QVariant();
    }
}

ModCacheModel::ModCacheModel(ModCache &cache, QObject *parent)
    : QAbstractListModel(parent), cache(cache) {}

int ModCacheModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return cache.mods().size();
}

int ModCacheModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant ModCacheModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= cache.mods().size())
        return QVariant();

    const CachedMod &cm = cache.mods()[index.row()];

    switch (index.column())
    {
        case 0:
            return Columns::modName(cm, role);
        case 1:
            return Columns::modId(cm, role);
        case 2:
            return Columns::modLatestVersion(cm, role);
        case 3:
            return Columns::modLatestUpdateTime(cm, role);
        default:
            return QVariant();
    }
}

QVariant ModCacheModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0:
                return QStringLiteral("Name");
            case 1:
                return QStringLiteral("ID");
            case 2:
                return QStringLiteral("Latest Version");
            case 3:
                return QStringLiteral("Update Time");
            default:
                return QVariant();
        }
    }
    else
        return QVariant();
}

}  // namespace iimodmanager
