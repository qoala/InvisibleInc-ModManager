#include "modelutil.h"
#include "modsmodel.h"

#include <QDateTime>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

namespace modelutil {
    typedef ModsModel::Status Status;

    Status modStatus(const CachedMod *cm, const InstalledMod *im, int role)
    {
        ModsModel::Status status = ModsModel::NO_STATUS;
        if (role != ModsModel::STATUS_ROLE)
            return status;

        if (cm)
        {
            status |= ModsModel::CACHED_STATUS;
            if (cm->info().isSteam())
                status |= ModsModel::STEAM_STATUS;
            if (!cm->downloaded())
                status |= ModsModel::NO_DOWNLOAD_STATUS;
            // TODO: Store recent SteamModInfo in ModCache and track CAN_DOWNLOAD_UPDATE_STATUS.

            if (cm->installedVersion())
            {
                status |= ModsModel::INSTALLED_STATUS;
                if (cm->installedVersion() != cm->latestVersion())
                    status |= ModsModel::CAN_INSTALL_UPDATE_STATUS;
            }
        }
        else if (im) // Should always be present if no cached mod.
        {
            // Installed, but uncached.
            status |= ModsModel::INSTALLED_STATUS;
            status |= ModsModel::NO_DOWNLOAD_STATUS;
            if (im->info().isSteam())
                status |= ModsModel::STEAM_STATUS;
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
            else if (role == ModsModel::STATUS_ROLE)
                return toVariant(baseStatus | ModsModel::UNLABELLED_STATUS);
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return version;
        else if (role == ModsModel::STATUS_ROLE)
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
            else if (role == ModsModel::STATUS_ROLE)
                return toVariant(baseStatus | ModsModel::UNLABELLED_STATUS);
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole)
            return *timestamp;
        else if (role == ModsModel::STATUS_ROLE)
            return toVariant(baseStatus);
        else
            return QVariant();
    }
}

}  // namespace iimodmanager
