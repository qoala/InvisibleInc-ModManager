#include "modelutil.h"
#include "cacheimportmodel.h"

#include <QAbstractItemModel>
#include <modcache.h>
#include <modspec.h>
#include <modlist.h>

namespace iimodmanager {

namespace ColumnData {
    typedef modelutil::Status Status;
}

CacheImportModel::CacheImportModel(const ModCache &cache, const ModList &modList, QObject *parent)
    : ModsModel(cache, modList, parent), previousEmptyState_(true)
{}

int CacheImportModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

int CacheImportModel::columnMax() const
{
    return COLUMN_MAX;
}

static bool isBase(int column)
{
    switch (column)
    {
    case CacheImportModel::NAME:
    case CacheImportModel::FOLDER:
    case CacheImportModel::INSTALLED_VERSION:
        return true;
    default:
        return false;
    }
}
inline bool isBase(const QModelIndex &index)
{
    return (!index.isValid()
            || (!index.parent().isValid() && isBase(index.column())));
}

static int toBaseColumn(int column) {
    switch (column)
    {
    case CacheImportModel::NAME:
        return ModsModel::NAME;
    case CacheImportModel::FOLDER:
        return ModsModel::ID;
    case CacheImportModel::INSTALLED_VERSION:
        return ModsModel::INSTALLED_VERSION;
    default:
        return column;
    }
}

inline QModelIndex toBaseColumn(const CacheImportModel *model, const QModelIndex &index) {
    return model->index(index.row(), toBaseColumn(index.column()));
}

QVariant CacheImportModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < cache.mods().size())
        return modelutil::nullData(role);
    if (isBase(index))
        return ModsModel::data(toBaseColumn(this, index), role);

    const CachedMod *cm;
    const InstalledMod *im;
    seekRow(index.row(), &cm, &im);

    modelutil::Status baseStatus = modelutil::modStatus(cm, im, role);

    if (!im)
        return modelutil::nullData(role);

    switch (index.column())
    {
    case ACTION:
        return QVariant();
    case ID:
        return QVariant();
    case STEAM_UPDATE_TIME:
        return QVariant();
    }
    return QVariant();
}

bool CacheImportModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < cache.mods().size())
        return false;
    if (index.column() == ACTION && role == Qt::CheckStateRole)
    {
        auto state = value.value<Qt::CheckState>();
        int row = index.row();
        const CachedMod *cm;
        const InstalledMod *im;
        seekRow(index.row(), &cm, &im);

        if (!im)
            return false;

        if (state == Qt::Checked)
        {
        }
        else if (state == Qt::Unchecked)
        {
        }

        reportImportChanged(im->id());
        return true;
    }
    else if (index.column() == ID && role == Qt::DisplayRole)
    {
        auto newId = value.toString();
        int row = index.row();
        const CachedMod *cm;
        const InstalledMod *im;

        reportImportChanged(im->id());
        return true;
    }
    return false;
}

QVariant CacheImportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();
    if (section == FOLDER && role == Qt::DisplayRole)
        return QStringLiteral("Folder");
    if (isBase(section))
        return ModsModel::headerData(toBaseColumn(section), orientation, role);

    switch (role)
    {
    case Qt::DisplayRole:
        switch (section)
        {
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
        case ID:
            return modelutil::MOD_ID_SORT;
        case STEAM_UPDATE_TIME:
            return modelutil::VERSION_TIME_SORT;
        }
        break;
    case modelutil::CANCEL_SORTING_ROLE:
        return QVariant::fromValue<QVector<int>>({ACTION, ID, STEAM_UPDATE_TIME});
    // TODO: case Qt::BackgroundRole:
    }

    return QVariant();
}

Qt::ItemFlags CacheImportModel::flags(const QModelIndex &index) const
{
    if (index.column() == ACTION)
    {
        Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        if (true)
            f |= Qt::ItemIsEnabled;
        return f;
    }

    return QAbstractItemModel::flags(index);
}

bool CacheImportModel::isEmpty() const
{
    return true;
}


void CacheImportModel::reportImportChanged(const QString &modId, int row)
{
    int startRow, endRow;

    if (modId.isEmpty())
    {
        // All uncached rows.
        startRow = cache.mods().size();
        endRow = rowCount() - 1;
    }
    else if (row != -1)
        startRow = endRow = row;
    else if ((endRow = rowOf(modId)) == -1)
        // Mod is not present.
        return;
    else
        // Update a single mod row.
        startRow = endRow;

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

} // namespace iimodmanager
