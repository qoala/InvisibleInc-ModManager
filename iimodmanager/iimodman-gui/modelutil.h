#ifndef MODELUTIL_H
#define MODELUTIL_H

#include "modsmodel.h"

namespace iimodmanager {

class CachedMod;
class InstalledMod;

namespace modelutil {

    inline QVariant toVariant(ModsModel::Status status)
    { return QVariant::fromValue<ModsModel::Status>(status); };

    //! Returns row-level status flags.
    ModsModel::Status modStatus(const CachedMod *cm, const InstalledMod *im = nullptr, int role = ModsModel::STATUS_ROLE);

    // Model data helpers.
    QVariant versionData(const QString &version, ModsModel::Status baseStatus, int role);
    QVariant versionTimeData(const CachedVersion *cv, ModsModel::Status baseStatus, int role);

} // namespace modelutil

}  // namespace iimodmanager

#endif // MODELUTIL_H
