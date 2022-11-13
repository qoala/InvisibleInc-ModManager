#ifndef MODSPECPREVIEWMODEL_H
#define MODSPECPREVIEWMODEL_H

#include "modsmodel.h"

#include <QAbstractItemModel>

namespace iimodmanager {

class ModCache;
class ModList;
class ModSpec;

//! Previews the effect of applying a given mod spec.
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
        COLUMN_MAX = TARGET_VERSION_TIME,
        COLUMN_COUNT = COLUMN_MAX+1,
    };

    struct PendingChange
    {
        enum ChangeType {
            NONE,
            INSTALL,
            REMOVE,
            UPDATE,
        };
        enum VersionPinning {
            //! Currently installed version.
            CURRENT,
            //! Latest available version.
            LATEST,
            //! Specific version.
            PINNED,
        };

        PendingChange(const QString &modId = QString())
            : modId(modId), versionId(), versionPin(LATEST), type(NONE) {};

        const QString modId;
        QString versionId;
        VersionPinning versionPin;
        ChangeType type;

        inline bool isValid() const { return !modId.isNull(); };
        inline bool isNone() const { return type == NONE; };
    };

    ModSpecPreviewModel(const ModCache &cache, const ModList &modList, QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setModSpec(const ModSpec &s);
    ModSpec modSpec() const;

protected:
    int columnMax() const override;

private:
    QHash<QString, PendingChange> pendingChanges;
};

}  // namespace iimodmanager

#endif // MODSPECPREVIEWMODEL_H
