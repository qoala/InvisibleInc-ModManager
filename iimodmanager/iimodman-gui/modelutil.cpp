#include "modelutil.h"

#include <QDateTime>
#include <optional>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

namespace modelutil {

    Status modStatus(const CachedMod *cm, const InstalledMod *im, int role)
    {
        Status status = NO_STATUS;
        if (role != STATUS_ROLE)
            return status;

        if (cm)
        {
            status |= CACHED_STATUS;
            if (cm->info().isSteam())
                status |= STEAM_STATUS;
            if (!cm->downloaded())
                status |= NO_DOWNLOAD_STATUS;
            // TODO: Store recent SteamModInfo in ModCache and track CAN_DOWNLOAD_UPDATE_STATUS.

            if (cm->installedVersion())
            {
                status |= INSTALLED_STATUS;
                if (cm->installedVersion() != cm->latestVersion())
                    status |= CAN_INSTALL_UPDATE_STATUS;
            }
        }
        else if (im) // Should always be present if no cached mod.
        {
            // Installed, but uncached.
            status |= INSTALLED_STATUS;
            status |= NO_DOWNLOAD_STATUS;
            if (im->info().isSteam())
                status |= STEAM_STATUS;
        }

        return status;
    }

    QVariant versionData(const QString &version, Status baseStatus, int role)
    {
        if (version.isEmpty())
        {
            if (role == Qt::DisplayRole)
                return QStringLiteral("-");
            else if (role == Qt::ToolTipRole)
                return QStringLiteral("(unlabeled)");
            else if (role == STATUS_ROLE)
                return toVariant(baseStatus | UNLABELLED_STATUS);
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return version;
        else if (role == STATUS_ROLE)
            return toVariant(baseStatus);
        else
            return QVariant();
    }

    QVariant versionTimeData(const CachedVersion *cv, Status baseStatus, int role)
    {
        if (!cv)
            // STATUS_ROLE handled by caller.
            return QVariant();

        const std::optional<QDateTime> timestamp = cv->timestamp();
        if (!timestamp)
        {
            if (role == Qt::DisplayRole)
                return cv->id();
            else if (role == STATUS_ROLE)
                return toVariant(baseStatus | UNLABELLED_STATUS);
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return *timestamp;
        else if (role == STATUS_ROLE)
            return toVariant(baseStatus);
        else
            return QVariant();
    }
}

}  // namespace iimodmanager
