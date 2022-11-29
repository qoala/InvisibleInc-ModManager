#ifndef MODDOWNLOADER_H
#define MODDOWNLOADER_H

#include "iimodman-lib_global.h"
#include "modmanconfig.h"

#include <QLoggingCategory>
#include <QObject>
#include <QNetworkAccessManager>
#include <QDateTime>


namespace iimodmanager {

class CachedVersion;
class ModCache;

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
class ApplicationVersionCall;

class IIMODMANLIBSHARED_EXPORT ModDownloader : public QObject
{
    Q_OBJECT

public:
    ModDownloader(const ModManConfig &config, QObject *parent = nullptr);

    ModInfoCall *fetchModInfo(const QString& id);
    ModInfoCall *modInfoCall();
    ModDownloadCall *downloadModVersion(ModCache &cache, const SteamModInfo& info);
    ModDownloadCall *modDownloadCall(ModCache &cache);
    ApplicationVersionCall *appVersionCall();

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
    inline const QString &errorDetail() const { return errorDetail_; };

signals:
    void finished();

private:
    const ModManConfig &config_;
    QNetworkAccessManager &qnam_;
    QString id_;
    SteamModInfo result_;
    QString errorDetail_;
};

class IIMODMANLIBSHARED_EXPORT ModDownloadCall : public QObject
{
    Q_OBJECT
    friend ModDownloader;

public:
    ModDownloadCall(const ModManConfig &config, QNetworkAccessManager &qnam, ModCache &cache, QObject *parent);

    void start(const SteamModInfo& info);

    inline const SteamModInfo& steamInfo() const { return info_; };
    const CachedVersion *resultVersion() const;
    inline const QString &errorDetail() const { return errorDetail_; };

signals:
    void finished();

private:
    const ModManConfig &config_;
    QNetworkAccessManager &qnam_;
    ModCache &cache_;

    SteamModInfo info_;
    QString resultVersionId_;
    QString errorDetail_;
};

//! Fetches details on the latest release of this mod manager application.
class IIMODMANLIBSHARED_EXPORT ApplicationVersionCall : public QObject
{
    Q_OBJECT
    friend ModDownloader;

public:
    ApplicationVersionCall(QNetworkAccessManager &qnam, QObject *parent);

    void startLatest();

    inline const QString &version() const { return version_; };
    inline const QString &url() const { return url_; };
    inline const QString &errorDetail() const { return errorDetail_; };

signals:
    void finished();

private:
    QNetworkAccessManager &qnam_;
    QString version_;
    QString url_;
    QString errorDetail_;
};

}  // namespace iimodmanager

#endif // MODDOWNLOADER_H
