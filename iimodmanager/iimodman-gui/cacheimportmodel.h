#ifndef CACHEIMPORTMODEL_H
#define CACHEIMPORTMODEL_H

#include "modelutil.h"
#include "modsmodel.h"

namespace iimodmanager {

class ModCache;
class ModList;
class ModSpec;
class SpecMod;

//! Tracks details for a pending import of installed mods to the cache.
class CacheImportModel : public ModsModel
{
    Q_OBJECT

public:
    enum Column
    {
        NAME, // from base
        FOLDER, // from base ID

        ACTION,

        ID,

        INSTALLED_VERSION, // from base

        STEAM_UPDATE_TIME,

        COLUMN_MAX = STEAM_UPDATE_TIME,
        COLUMN_COUNT = COLUMN_MAX+1,
    };

    CacheImportModel(const ModCache &cache, const ModList &modList, QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    //! Returns true if there is anything to import.
    bool isEmpty() const;

signals:
    void isEmptyChanged(bool newValue) const;

protected:
    int columnMax() const override;

private:
    //! Report the the visible model has changed.
    //! Pass the updated mod ID for a single mod; leave blank for all mods.
    void reportImportChanged(const QString &modId = QString(), int row = -1);

    // Used to report ::isEmptyChanged.
    mutable bool previousEmptyState_;
};

}  // namespace iimodmanager

#endif // CACHEIMPORTMODEL_H

