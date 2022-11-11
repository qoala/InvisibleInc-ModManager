#ifndef MODCACHEMODEL_H
#define MODCACHEMODEL_H

#include <QAbstractListModel>
#include <modcache.h>

namespace iimodmanager {


class ModCacheModel : public QAbstractListModel
{
    Q_OBJECT

    enum Columns
    {
        NAME = 0,
        ID = 1,
        LATEST_VERSION = 2,
        UPDATE_TIME = 3,

        COLUMN_MIN = 0,
        COLUMN_MAX = 3,
        COLUMN_COUNT = 4,
    };

public:
    ModCacheModel(ModCache &cache, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private slots:
    void sourceAboutToAppendMods(const QStringList &modIds);
    void sourceAppendedMods();
    void sourceAboutToRefresh(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint);
    void sourceRefreshed(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint);
    void sourceMetadataChanged(const QStringList &modIds, const QList<int> &modIdxs);

private:
    ModCache &cache;
    // Temporary storage between aboutToRefresh and refresh signals.
    QModelIndexList savedPersistentIndexes;
    QVector<QString> savedPersistentMappings;
};

}  // namespace iimodmanager

#endif // MODCACHEMODEL_H
