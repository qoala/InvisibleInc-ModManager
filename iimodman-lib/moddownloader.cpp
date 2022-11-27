#include "moddownloader.h"
#include "modcache.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTemporaryFile>

namespace iimodmanager {

Q_LOGGING_CATEGORY(steamAPI, "steamapi", QtWarningMsg)


ModDownloader::ModDownloader(const ModManConfig &config, QObject *parent)
    : QObject(parent), config_(config)
{}

ModInfoCall *ModDownloader::fetchModInfo(const QString &id)
{
    ModInfoCall *call = new ModInfoCall(config_, qnam_, this);
    call->start(id);
    return call;
}

ModInfoCall *ModDownloader::modInfoCall()
{
    ModInfoCall *call = new ModInfoCall(config_, qnam_, this);
    return call;
}

ModDownloadCall *ModDownloader::downloadModVersion(ModCache &cache, const SteamModInfo &info)
{
    ModDownloadCall *call = new ModDownloadCall(config_, qnam_, cache, this);
    call->start(info);
    return call;
}

ModDownloadCall *ModDownloader::modDownloadCall(ModCache &cache)
{
    ModDownloadCall *call = new ModDownloadCall(config_, qnam_, cache, this);
    return call;
}

static SteamModInfo parseSteamModInfo(const QByteArray rawData, const QString &debugContext, QString *errorInfo = nullptr)
{
        SteamModInfo result;

        QJsonParseError errors;
        const QJsonDocument json = QJsonDocument::fromJson(rawData, &errors);

        if (json.isNull() || !json.isObject())
        {
            qCWarning(steamAPI).noquote() << debugContext << "Invalid response from Steam API:" << errors.errorString();
            if (errorInfo)
                *errorInfo = QStringLiteral("Invalid response from Steam API: %1").arg(errors.errorString());
            return result;
        }
        if (!json.isObject())
        {
            qCWarning(steamAPI).noquote() << debugContext << "Invalid response from Steam API: Not a JSON object.";
            if (errorInfo)
                *errorInfo = QStringLiteral("Invalid response from Steam API: Not a JSON object.");
            return result;
        }

        const QJsonObject response = json.object().value("response").toObject();
        if (!response.value("resultcount").isDouble() || response.value("resultcount").toInt(0) < 1)
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: No results.";
            if (errorInfo)
                *errorInfo = QStringLiteral("No results from Steam API.");
            return result;
        }
        assert(response.value("resultcount").toInt(0) >= 1);
        const QJsonObject fileDetail = response.value("publishedfiledetails").toArray().at(0).toObject();

        int appId = fileDetail.value("consumer_app_id").toInt();
        if (appId != 243970)
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: Mod is for non-Invisible-Inc app:" << appId;
            if (errorInfo)
                *errorInfo = QStringLiteral("Mod is for non-Invisible-Inc app: ID %1").arg(appId);
            return result;
        }
        QString filename = fileDetail.value("filename").toString();
        if (!filename.endsWith(".zip"))
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: File is not a .zip, possibly not a Mod:" << filename;
            if (errorInfo)
                *errorInfo = QStringLiteral("File is not a .zip, possibly not a Mod: %1").arg(filename);
            return result;
        }

        QString workshopId = fileDetail.value("publishedfileid").toString();
        QString title = fileDetail.value("title").toString();
        QString description = fileDetail.value("description").toString();
        QString downloadUrl = fileDetail.value("file_url").toString();
        QDateTime lastUpdated = QDateTime::fromSecsSinceEpoch(fileDetail.value("time_updated").toInt(), Qt::UTC);

        if (workshopId.isEmpty())
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: no publishedfileid";
            if (errorInfo)
                *errorInfo = QStringLiteral("Steam API response is missing publishedfileid");
            return result;
        }
        if (downloadUrl.isEmpty())
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: no file_url";
            if (errorInfo)
                *errorInfo = QStringLiteral("Steam API response is missing file_url");
            return result;
        }

        result.id = workshopId;
        result.title = title;
        result.description = description;
        result.downloadUrl = downloadUrl;
        result.lastUpdated = lastUpdated;
        return result;
}

ModInfoCall::ModInfoCall(const ModManConfig &config, QNetworkAccessManager &qnam, QObject *parent)
    : QObject(parent), config_(config), qnam_(qnam)
{}

void ModInfoCall::start(const QString &id)
{
    const QString callDebugInfo = QString("ModInfo(%1)").arg(id);
    id_ = id;

    static const QString getPublishedFileDetails = QStringLiteral("https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/");
    QNetworkRequest request(getPublishedFileDetails);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QString postData;
    postData.append("itemcount=1&");
    postData.append("publishedfileids[0]=").append(id);

    qCDebug(steamAPI).noquote() << callDebugInfo << "Request Start";
    QNetworkReply *reply = qnam_.post(request, postData.toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, callDebugInfo, reply]
    {
        qCDebug(steamAPI).noquote() << callDebugInfo << "Request End";
        result_ = parseSteamModInfo(reply->readAll(), callDebugInfo, &errorDetail_);
        reply->deleteLater();
        emit finished();
    });
}

ModDownloadCall::ModDownloadCall(const ModManConfig &config, QNetworkAccessManager &qnam, ModCache &cache, QObject *parent)
    : QObject(parent), config_(config), qnam_(qnam), cache_(cache)
{}

void ModDownloadCall::start(const SteamModInfo &info)
{
    const QString callDebugInfo = QString("ModDownload(%1,%2)").arg(info.id, info.lastUpdated.toString(Qt::ISODate));
    info_ = info;

    QTemporaryFile *zipFile = new QTemporaryFile(this);
    if (!zipFile->open())
    {
        qFatal("%s Failed to create temporary file", callDebugInfo.toUtf8().constData());
    }

    qCDebug(steamAPI).noquote() << callDebugInfo << "Request Start";
    QNetworkRequest request(info.downloadUrl);
    QNetworkReply *reply = qnam_.get(request);
    connect(reply, &QIODevice::readyRead, this, [zipFile, callDebugInfo, reply]
    {
        QByteArray data = reply->readAll();
        qCDebug(steamAPI).noquote() << callDebugInfo << "Chunk Update" << data.size() << "bytes";
        zipFile->write(data);
    });

    connect(reply, &QNetworkReply::finished, this, [this, info, zipFile, callDebugInfo, reply]
    {
        QByteArray data = reply->readAll();
        qCDebug(steamAPI).noquote() << callDebugInfo << "Request End" << data.size() << "bytes";
        zipFile->write(reply->readAll());
        reply->deleteLater();

        zipFile->seek(0);
        const CachedVersion *v = cache_.addZipVersion(info_, *zipFile, &errorDetail_);
        if (v)
            resultVersionId_ = v->id();
        else
            resultVersionId_.clear();
        zipFile->deleteLater();

        emit finished();
    });
}

const CachedVersion *ModDownloadCall::resultVersion() const
{
   const CachedMod *m = cache_.mod(info_.modId());
   if (m)
       return m->version(resultVersionId_);
   return nullptr;
}

}  // namespace iimodmanager
