#ifndef MODDOWNLOADER_H
#define MODDOWNLOADER_H

#include "iimodman-lib_global.h"
#include "modmanconfig.h"

#include <QFuture>
#include <QLoggingCategory>
#include <QObject>
#include <QNetworkAccessManager>
#include <QDateTime>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(steamAPI)

struct IIMODMANLIBSHARED_EXPORT SteamModInfo
{
    QString id;
    QString title;
    QString description;
    QString downloadUrl;
    QDateTime lastUpdated;
};

class ModInfoCall;

class IIMODMANLIBSHARED_EXPORT ModDownloader : public QObject
{
    Q_OBJECT

public:
    ModDownloader(const ModManConfig &config, QObject *parent = nullptr);

    ModInfoCall *fetchModInfo(const QString& id);
    ModInfoCall *modInfoCall();
    QFuture<QString> downloadModVersion(const SteamModInfo& info);

private:
    const ModManConfig &config_;
    QNetworkAccessManager qnam_;
};

class IIMODMANLIBSHARED_EXPORT ModInfoCall : public QObject
{
    Q_OBJECT
    friend ModDownloader;

public:
    ModInfoCall(const ModManConfig &config, QNetworkAccessManager &qnam, QObject *parent);

    void start(const QString& id);

    inline const SteamModInfo &result() { return result_; };

signals:
    void finished();

private:
    const ModManConfig &config_;
    QNetworkAccessManager &qnam_;
    SteamModInfo result_;
};

}  // namespace iimodmanager

#endif // MODDOWNLOADER_H
