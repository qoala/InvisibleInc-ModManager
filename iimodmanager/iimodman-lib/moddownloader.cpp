#include "moddownloader.h"
#include "modcache.h"

#include <JlCompress.h>
#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTemporaryFile>

namespace iimodmanager {

Q_LOGGING_CATEGORY(steamAPI, "steamapi", QtWarningMsg)

const QString getPublishedFileDetails = QStringLiteral("https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/");

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

ModDownloadCall *ModDownloader::downloadModVersion(const SteamModInfo &info)
{
    ModDownloadCall *call = new ModDownloadCall(config_, qnam_, this);
    call->start(info);
    return call;
}

ModDownloadCall *ModDownloader::modDownloadCall()
{
    ModDownloadCall *call = new ModDownloadCall(config_, qnam_, this);
    return call;
}

SteamModInfo parseSteamModInfo(const QByteArray rawData, const QString &debugContext)
{
        SteamModInfo result;

        QJsonParseError errors;
        const QJsonDocument json = QJsonDocument::fromJson(rawData, &errors);

        if (json.isNull() || !json.isObject())
        {
            qCWarning(steamAPI).noquote() << debugContext << "Invalid response from Steam API:" << errors.errorString();
            return result;
        }
        if (!json.isObject())
        {
            qCWarning(steamAPI).noquote() << debugContext << "Invalid response from Steam API: Not a JSON object.";
            return result;
        }

        const QJsonObject response = json.object().value("response").toObject();
        if (!response.value("resultcount").isDouble() || response.value("resultcount").toInt(0) < 1)
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: No results.";
            return result;
        }
        assert(response.value("resultcount").toInt(0) >= 1);
        const QJsonObject fileDetail = response.value("publishedfiledetails").toArray().at(0).toObject();

        int appId = fileDetail.value("consumer_app_id").toInt();
        if (appId != 243970)
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: Mod is for non-Invisible-Inc app:" << appId;
            return result;
        }
        QString filename = fileDetail.value("filename").toString();
        if (!filename.endsWith(".zip"))
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: File is not a .zip, possibly not a Mod:" << filename;
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
            return result;
        }
        if (downloadUrl.isEmpty())
        {
            qCWarning(steamAPI).noquote() << debugContext << "Steam API: no file_url";
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
        result_ = parseSteamModInfo(reply->readAll(), callDebugInfo);
        reply->deleteLater();
        emit finished();
    });
}

ModDownloadCall::ModDownloadCall(const ModManConfig &config, QNetworkAccessManager &qnam, QObject *parent)
    : QObject(parent), config_(config), qnam_(qnam)
{}

void ModDownloadCall::start(const SteamModInfo &info)
{
    const QString callDebugInfo = QString("ModDownload(%1,%2)").arg(info.id, info.lastUpdated.toString(Qt::ISODate));
    info_ = info;

    resultPath_ = ModCache::modVersionPath(config_, info.modId(), info.lastUpdated);
    QDir modVersionDir(resultPath());

    if (modVersionDir.exists() && !modVersionDir.isEmpty())
    {
        qCWarning(steamAPI).noquote() << callDebugInfo << "Mod version folder is not empty. Will replace existing.";
    }

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

        QDir outputDir(resultPath());
        if (outputDir.exists() && !outputDir.isEmpty())
        {
            if (outputDir.exists("modinfo.txt"))
            {
                qCDebug(steamAPI).noquote() << callDebugInfo << "Deleting existing" << outputDir.absolutePath();
                outputDir.removeRecursively();
            }
            else
            {
                qFatal("%s Output directory non-empty and not a mod folder: %s",
                       callDebugInfo.toUtf8().constData(),
                       outputDir.absolutePath().toUtf8().constData());
            }
        }

        qCDebug(steamAPI).noquote() << callDebugInfo << "Unzip Start" << outputDir.absolutePath();
        zipFile->seek(0);
        JlCompress::extractDir(zipFile, outputDir.absolutePath());
        zipFile->deleteLater();
        qCDebug(steamAPI).noquote() << callDebugInfo << "Unzip End";

        emit finished();
    });
}

}  // namespace iimodmanager
