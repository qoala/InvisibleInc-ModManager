#ifndef MODDOWNLOADER_H
#define MODDOWNLOADER_H

#include "iimodman-lib_global.h"
#include "modmanconfig.h"

#include <QFuture>
#include <QObject>
#include <QNetworkAccessManager>
#include <QDateTime>

namespace iimodmanager {

struct IIMODMANLIBSHARED_EXPORT SteamModInfo
{
    QString id;
    QString title;
    QString description;
    QString downloadUrl;
    QDateTime lastUpdated;
};

class IIMODMANLIBSHARED_EXPORT ModDownloader : public QObject
{
    Q_OBJECT

public:
    ModDownloader(const ModManConfig &config, QObject *parent = nullptr);

    QFuture<SteamModInfo> fetchModInfo(const QString& id);
    QFuture<QString> downloadModVersion(const SteamModInfo& info);

private:
    const ModManConfig &config_;
    QNetworkAccessManager qnam_;
};

}  // namespace iimodmanager

#endif // MODDOWNLOADER_H
