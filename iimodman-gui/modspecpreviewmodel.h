#ifndef MODSPECPREVIEWMODEL_H
#define MODSPECPREVIEWMODEL_H

#include "modsmodel.h"

#include <QAbstractItemModel>
#include <optional>
#include <modspec.h>

namespace iimodmanager {

class ModCache;
class ModList;
class ModSpec;
class SpecMod;

//! Previews the effect of applying a given mod specification.
class ModSpecPreviewModel : public ModsModel
{
    Q_OBJECT

public:
    enum Column
    {
        NAME, // from base
        ID, // from base

        INSTALLED_ALIAS, // from base
        TARGET_ALIAS,

        ACTION,

        INSTALLED_VERSION, // from base
        TARGET_VERSION,
        LATEST_VERSION, // from base

        INSTALLED_VERSION_TIME, // from base
        TARGET_VERSION_TIME,
        CACHE_UPDATE_TIME, // from base

        COLUMN_MAX = CACHE_UPDATE_TIME,
        COLUMN_COUNT = COLUMN_MAX+1,
    };

    struct PendingChange
    {
        enum ChangeType {
            //! No change. An uninstalled mod remains so. An installed mod remains installed.
            //! The latter case may be further specified with PIN_CURRENT or PIN_LATEST to modify how this change reacts to cache changes.
            NONE,
            //! Like NONE, but specifically pinning the current installed version.
            PIN_CURRENT,
            //! Like NONE, but specifically pinning the latest available version.
            //! Must have LATEST pin type.
            PIN_LATEST,
            INSTALL,
            REMOVE,
            //! Remove the currently installed mod, and re-install it with a different alias.
            //! The installed version may change.
            RE_ALIAS,
            //! Install a different version of the mod (not necessarily newer).
            //! The mod will be installed under the same alias/ID as the current installation.
            UPDATE,

            ACTIVE_CHANGE_MIN = INSTALL,
        };
        //! How this change specifies a version.
        //! Controls how this pending change is affected by cache/installed updates.
        enum VersionPinning {
            //! Currently installed version.
            CURRENT,
            //! Latest available version.
            LATEST,
            //! Specific version.
            PINNED,
        };

        QString modId;
        QString modName;
        QString alias;
        QString versionId;
        VersionPinning versionPin;
        ChangeType type;

        PendingChange(const QString &modId = QString())
            : modId(modId), versionPin(CURRENT), type(NONE) {};

        inline bool isValid() const { return !modId.isNull(); };
        inline bool isNone() const { return type == NONE; };
        inline bool isActive() const { return type >= ACTIVE_CHANGE_MIN; }
    };

    ModSpecPreviewModel(const ModCache &cache, const ModList &modList, QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    //! Returns a specification of the currently previewed target mods.
    //! Versions are only specified for pinned mods.
    QList<SpecMod> modSpec() const;
    //! Returns a specification of the currently previewed target mods.
    //! All versions are specified.
    QList<SpecMod> versionedModSpec() const;
    void prepareChanges(QList<SpecMod> *toAddMods, QList<SpecMod> *toUpdateMods, QList<InstalledMod> *toRemoveMods) const;
    //! Returns true if there any differences compared to the currently installed state.
    //! \sa ::revert
    bool isEmpty() const;

signals:
    void textOutput(QString value) const;

    void isEmptyChanged(bool newValue) const;

public slots:
    //! Resets the pending mods to the currently installed state.
    //!
    //! Submitting changes should be handled outside of this model,
    //! instead of by calling ::submit.
    //!
    //! \sa ::isEmpty
    void revert() override;

    //! Replaces pending changes with an exact sync to the given mod specification.
    //! \sa ::revert
    void setModSpec(const QList<SpecMod> &specMods);
    //! Overwrites pending changes for mods in the given mod specification.
    //! Mods not in the specification will be unaffected.
    void insertModSpec(const QList<SpecMod> &specMods);

    void setLock(bool locked);

protected:
    int columnMax() const override;
    void reportCacheChanged(const std::function<void ()> &cb, const QString &modId = QString()) override;
    void reportAllChanged(const std::function<void ()> &cb, const QString &modId = QString()) override;

private:
    QHash<QString, PendingChange> pendingChanges;
    //! True if an action is currently modifying the cached/installed mods.
    bool isLocked_;

    //! Most recently exported mod spec.
    mutable QList<SpecMod> modSpec_;
    mutable QList<SpecMod> versionedModSpec_;
    //! True if modSpec_ needs to be regenerated.
    mutable bool dirty;

    // Used to report ::isEmptyChanged.
    mutable bool previousEmptyState_;

    inline bool isLocked() const { return isLocked_; };

    inline const PendingChange pendingChange(const QString &modId) const
    { return pendingChanges.value(modId, PendingChange(modId)); }
    //! Populates the cached mod, installed mod, and pending change.
    //! If not present, a default pending change is provided.
    const PendingChange seekPendingRow(int row, const CachedMod **cmOut, const InstalledMod **imOut) const;
    //! Populates the cached mod, installed mod, and pending change.
    //! If the mod doesn't exist, nullptr is returned.
    //! If a pending action is not present, a default is inserted and provided for editing.
    PendingChange *seekMutablePendingRow(int row, const CachedMod **cmOut, const InstalledMod **imOut);

    //! Marks the mod spec as needing to be regenerated.
    inline void setDirty() const { dirty = true; };
    //! Report that the visible model has changed.
    //! Pass the updated mod ID for a single mod; leave blank for all mods.
    void reportSpecChanged(int row = -1, bool modifiedByView = false);

    std::optional<PendingChange> toPendingChange(const SpecMod &specMod) const;
    void generateModSpec() const;
    void refreshPendingChange(PendingChange &pc);
    void refreshPendingChanges();
};

}  // namespace iimodmanager

#endif // MODSPECPREVIEWMODEL_H
