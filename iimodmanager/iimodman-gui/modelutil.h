#ifndef MODELUTIL_H
#define MODELUTIL_H

#include <QFlags>
#include <QVariant>

namespace iimodmanager {

class CachedMod;
class CachedVersion;
class InstalledMod;

namespace modelutil {
    Q_NAMESPACE

    enum UserDataRole
    {
        //! Used on an item to determine row and field properties in a Status flag set. (Status)
        STATUS_ROLE = 0x100,
        //! Used on a header to identify the column's item type for sorting. (SortType)
        //! Or used on an item when the column has ROLE_SORT, to return a sortable value. (int)
        SORT_ROLE = 0x101,
        //! Used on a header when data is modified to identify columns that depend on this item.
        //! If the current sort is one of those columns, then the current sorting is cancelled.
        //! (QVector<int>)
        CANCEL_SORTING_ROLE = 0x102,
    };

    //! Properties of the given row (or for the highest flags, the specific item).
    enum StatusFlag
    {
        NO_STATUS = 0,
        //! The row is a mod that's installed.
        INSTALLED_STATUS = 1,
        //! The row is a mod that's in the cache.
        CACHED_STATUS = 2,
        //! The row is a steam workshop mod.
        STEAM_STATUS = 4,

        //! The row is a mod that's not downloaded.
        NO_DOWNLOAD_STATUS = 0x10,
        //! The row is a mod that's not downloaded.
        CAN_DOWNLOAD_UPDATE_STATUS = 0x20,
        //! The row is a mod that's not downloaded.
        CAN_INSTALL_UPDATE_STATUS = 0x40,

        //! The requested column is null for the requested row.
        //! Example: If a given mod version does not exist, it is NULL.
        NULL_STATUS = 0x100,
        //! The requested column is unlabelled for the requested mod.
        //! Example: If a given mod version exists, but does not declare a 'version' string, it is UNLABELLED.
        UNLABELLED_STATUS = 0x200,
    };
    Q_DECLARE_FLAGS(Status, StatusFlag)
    Q_FLAG_NS(Status)

    //! The value type of this header's column, for sorting.
    enum SortType
    {
        NORMAL_SORT,
        //! String, sorted as a mod ID.
        MOD_ID_SORT,
        //! Int, obtained from the SORT_ROLE role.
        ROLE_SORT,
        //! String, sorted as a version string.
        VERSION_SORT,
        //! DateTime, with a fallback to String.
        VERSION_TIME_SORT,
    };


    inline QVariant toVariant(Status status)
    { return QVariant::fromValue<Status>(status); };

    //! Returns row-level status flags.
    Status modStatus(const CachedMod *cm, const InstalledMod *im = nullptr, int role = STATUS_ROLE);

    // Model data helpers.
    QVariant versionData(const QString &version, Status baseStatus, int role);
    QVariant versionTimeData(const CachedVersion *cv, Status baseStatus, int role);

} // namespace modelutil

Q_DECLARE_OPERATORS_FOR_FLAGS(modelutil::Status)

}  // namespace iimodmanager

#endif // MODELUTIL_H
