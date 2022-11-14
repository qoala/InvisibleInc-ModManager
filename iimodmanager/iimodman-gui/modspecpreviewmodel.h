#ifndef MODSPECPREVIEWMODEL_H
#define MODSPECPREVIEWMODEL_H

#include "modsmodel.h"

#include <QAbstractItemModel>
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
        // Pre-base columns.

        // Base columns.
        NAME,
        ID,
        INSTALLED_VERSION,
        INSTALLED_VERSION_TIME,
        LATEST_VERSION,
        CACHE_UPDATE_TIME,

        // Post-base columns.
        ACTION,
        TARGET_VERSION,
        TARGET_VERSION_TIME,

        BASE_COLUMN_MIN = NAME,
        BASE_COLUMN_MAX = CACHE_UPDATE_TIME,
        NEW_COLUMN_MIN = ACTION,
        NEW_COLUMN_MAX = TARGET_VERSION_TIME,
        COLUMN_MAX = TARGET_VERSION_TIME,
        COLUMN_COUNT = COLUMN_MAX+1,
    };

    struct PendingChange
    {
        enum ChangeType {
            NONE,
            //! Like NONE, but specifically pinning the current installed version.
            PIN_CURRENT,
            //! Like NONE, but specifically pinning the latest available version.
            //! Must have LATEST pin type.
            PIN_LATEST,
            INSTALL,
            REMOVE,
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
        QString versionId;
        VersionPinning versionPin;
        ChangeType type;

        PendingChange(const QString &modId = QString())
            : modId(modId), versionId(), versionPin(CURRENT), type(NONE) {};

        inline bool isValid() const { return !modId.isNull(); };
        inline bool isNone() const { return type == NONE; };
    };

    ModSpecPreviewModel(const ModCache &cache, const ModList &modList, QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    //! Returns a specification of the currently previewed target mods.
    QList<SpecMod> modSpec() const;
    //! Returns true if there any differences compared to the currently installed state.
    //! \sa ::revert
    bool isEmpty() const;

signals:
    void textOutput(QString value) const;

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

protected:
    int columnMax() const override;
    void reportAllChanged(const std::function<void ()> &cb) override;

private:
    QHash<QString, PendingChange> pendingChanges;

    //! Most recently exported mod spec.
    mutable QList<SpecMod> modSpec_;
    //! True if modSpec_ needs to be regenerated.
    mutable bool dirty;

    inline const PendingChange pendingChange(const QString &modId) const
    { return pendingChanges.value(modId, PendingChange(modId)); }

    //! Marks the mod spec as needing to be regenerated.
    inline void setDirty() const { dirty = true; };
    //! Report the the visible model has changed.
    //! Pass the updated mod ID for a single mod; leave blank for all mods.
    void reportSpecChanged(const QString &modId = QString());

    std::optional<PendingChange> toPendingChange(const SpecMod &specMod) const;
    void generateModSpec() const;
    void refreshPendingChanges();
};

}  // namespace iimodmanager

#endif // MODSPECPREVIEWMODEL_H
