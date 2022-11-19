#ifndef CACHEIMPORTMODEL_H
#define CACHEIMPORTMODEL_H

#include "modelutil.h"
#include "modsmodel.h"

#include <utility>
#include <moddownloader.h>

namespace iimodmanager {

class ModCache;
class ModList;
class ModSpec;
class SpecMod;
struct SteamModInfo;

//! Tracks details for a pending import of installed mods to the cache.
class CacheImportModel : public ModsModel
{
    Q_OBJECT

public:
    enum Column
    {
        NAME, // from base

        ACTION,
        ID,

        FOLDER,
        INSTALLED_VERSION, // from base
        STEAM_UPDATE_TIME,

        COLUMN_MAX = STEAM_UPDATE_TIME,
        COLUMN_COUNT = COLUMN_MAX+1,
    };
    struct PendingImport
    {
        enum ImportStatus
        {
            NONE,
            //! Already in the cache.
            IN_CACHE,
            //! Workshop ID didn't correspond to an II mod. Don't allow copying with this ID.
            NOT_MOD,
            //! Steam info is pending fetch.
            PENDING,

            //! Does not have a workshop mod ID. Can be copied as a non-steam mod.
            NOT_WORKSHOP,
            //! Can be imported.
            DOWNLOAD_AVAILABLE,

            //! Will be imported as-is.
            IMPORT_COPY,
            //! Will be imported as a workshop download.
            IMPORT_DOWNLOAD,
        };

        //! Installed mod ID.
        QString installedId;
        //! Target mod ID.
        QString modId;
        //! Steam workshop ID, if available for current modId.
        QString steamId;
        ImportStatus status;

        PendingImport() : status(NONE) {}

        inline bool isValid() const { return !installedId.isEmpty(); }
        inline bool isAvailable() const { return status >= NOT_WORKSHOP; }
        inline bool isActive() const { return status >= IMPORT_COPY; }
        void updateStatus(const CachedMod *cm, const InstalledMod *im, const SteamModInfo *steamInfo);
        bool activate();
        bool deactivate();
    };

    CacheImportModel(const ModCache &cache, const ModList &modList, ModInfoCall *steamInfoCall, QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void prepareChanges(QList<SteamModInfo> *toDownloadMods, QList<std::pair<QString, QString>> *toCopyMods) const;

    //! Returns true if there is anything to import.
    bool isEmpty() const;

signals:
    void textOutput(QString value) const;

    void isEmptyChanged(bool newValue) const;

protected:
    int columnMax() const override;

private slots:
    void steamInfoFinished();

private:
    //! Pending imports, by installed mod ID.
    QHash<QString, PendingImport> pendingImports;
    //! Steam mod info results, by steam workshop ID.
    QHash<QString, SteamModInfo> infoResults;
    ModInfoCall *steamInfoCall;
    bool steamInfoIdle;

    // Used to report ::isEmptyChanged.
    mutable bool previousEmptyState_;

    inline const PendingImport pendingImport(const QString &installedId) const
    { return pendingImports.value(installedId); }
    //! Populates the cached mod, installed mod, and pending change.
    //! If not present, an invalid pending import is provided.
    const PendingImport seekPendingRow(int row, const InstalledMod **imOut) const;
    //! Populates the cached mod, installed mod, and pending change.
    //! If the mod doesn't exist, nullptr is returned.
    //! If a pending action is not present, a default is inserted and provided for editing.
    PendingImport *seekMutablePendingRow(int row, const InstalledMod **imOut);

    void nextInfo();

    //! Report that the visible model has changed.
    //! Pass the updated row for a single mod; leave blank for all mods.
    void reportImportChanged(int row = -1, bool modifiedByView = false);
};

}  // namespace iimodmanager

#endif // CACHEIMPORTMODEL_H

