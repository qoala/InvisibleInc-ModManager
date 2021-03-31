#ifndef MODDOWNLOADER_H
#define MODDOWNLOADER_H

#include "iimodman-lib_global.h"
#include "modmanconfig.h"

#include <QLoggingCategory>
#include <QObject>
#include <QNetworkAccessManager>
#include <QDateTime>


namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(steamAPI)

//! Details about a mod from the Steam API.
struct IIMODMANLIBSHARED_EXPORT SteamModInfo
{
    QString id;
    QString title;
    QString description;
    QString downloadUrl;
    QDateTime lastUpdated;

    inline const QString modId() const { return QStringLiteral("workshop-%1").arg(id); };
    inline bool valid() const { return !id.isEmpty(); };
};

class ModInfoCall;
class ModDownloadCall;

class IIMODMANLIBSHARED_EXPORT ModDownloader : public QObject
{
    Q_OBJECT

public:
    ModDownloader(const ModManConfig &config, QObject *parent = nullptr);

    ModInfoCall *fetchModInfo(const QString& id);
    ModInfoCall *modInfoCall();
    ModDownloadCall *downloadModVersion(const SteamModInfo& info);
    ModDownloadCall *modDownloadCall();

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

    inline const QString &workshopId() const { return id_; };
    inline const SteamModInfo &result() const { return result_; };

signals:
    void finished();

private:
    const ModManConfig &config_;
    QNetworkAccessManager &qnam_;
    QString id_;
    SteamModInfo result_;
};

class IIMODMANLIBSHARED_EXPORT ModDownloadCall : public QObject
{
    Q_OBJECT
    friend ModDownloader;

public:
    ModDownloadCall(const ModManConfig &config, QNetworkAccessManager &qnam, QObject *parent);

    void start(const SteamModInfo& info);

    inline const SteamModInfo& modInfo() const { return info_; };
    inline const QString &resultPath() const { return resultPath_; };

signals:
    void finished();

private:
    const ModManConfig &config_;
    QNetworkAccessManager &qnam_;
    SteamModInfo info_;
    QString resultPath_;
};

}  // namespace iimodmanager

#endif // MODDOWNLOADER_H
