#ifndef MODSMODEL_H
#define MODSMODEL_H

#include <QAbstractListModel>
#include <modcache.h>

namespace iimodmanager {

class ModsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Column
    {
        NAME = 0,
        ID = 1,
        LATEST_VERSION = 2,
        CACHE_UPDATE_TIME = 3,

        COLUMN_MIN = 0,
        COLUMN_MAX = 3,
        COLUMN_COUNT = 4,
    };
    enum UserDataRole
    {
        STATUS_ROLE = 0x100,
    };
    enum StatusFlag
    {
        NO_STATUS = 0,
        //! The row is a mod that's installed.
        INSTALLED_STATUS = 1,
        //! The row is a mod that's not downloaded.
        NO_DOWNLOAD_STATUS = 2,
        //! The row is a mod that's not downloaded.
        CAN_INSTALL_UPDATE_STATUS = 4,
        //! The row is a mod that's not downloaded.
        CAN_DOWNLOAD_UPDATE_STATUS = 8,

        //! The requested column is unlabelled for the requested mod.
        UNLABELLED_STATUS = 0x100,
    };
    Q_DECLARE_FLAGS(Status, StatusFlag)
    Q_FLAG(Status)

    ModsModel(ModCache &cache, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private slots:
    void cacheAboutToAppendMods(const QStringList &modIds);
    void cacheAppendedMods();
    void cacheAboutToRefresh(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint);
    void cacheRefreshed(const QStringList &modIds, const QList<int> &modIdxs, ModCache::ChangeHint hint);
    void cacheMetadataChanged(const QStringList &modIds, const QList<int> &modIdxs);

private:
    ModCache &cache;
    // Temporary storage between aboutToRefresh and refresh signals.
    QModelIndexList savedPersistentIndexes;
    QVector<QString> savedPersistentMappings;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ModsModel::Status)

}  // namespace iimodmanager

#endif // MODSMODEL_H
