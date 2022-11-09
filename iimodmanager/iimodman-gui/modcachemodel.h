#ifndef MODCACHEMODEL_H
#define MODCACHEMODEL_H

#include <QAbstractListModel>
#include <modcache.h>

namespace iimodmanager {


class ModCacheModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ModCacheModel(ModCache &cache, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    ModCache &cache;
};

}  // namespace iimodmanager

#endif // MODCACHEMODEL_H
