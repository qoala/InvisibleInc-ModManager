#include "moddownloader.h"

#include <JlCompress.h>
#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QTextStream>

namespace iimodmanager {

const QString getPublishedFileDetails = QStringLiteral("https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/");

ModDownloader::ModDownloader(const ModManConfig &config, QObject *parent)
    : QObject(parent), config_(config)
{}

SteamModInfo parseSteamModInfo(const QByteArray rawData)
{
        QJsonParseError errors;
        const QJsonDocument json = QJsonDocument::fromJson(rawData, &errors);

        if (json.isNull())
        {
            // TODO: error handling
            QTextStream cerr(stderr);
            cerr << "JSON Parse Error: " << errors.errorString() << Qt::endl;
            cerr << rawData << Qt::endl;
        }

        assert(!json.isNull());
        assert(json.isObject());

        const QJsonObject response = json.object().value("response").toObject();
        assert(response.value("resultcount").toInt(0) >= 1);
        const QJsonObject fileDetail = response.value("publishedfiledetails").toArray().at(0).toObject();

        SteamModInfo result;

        result.id = fileDetail.value("publishedfileid").toString();
        result.title = fileDetail.value("title").toString();
        result.description = fileDetail.value("description").toString();
        result.downloadUrl = fileDetail.value("file_url").toString();
        result.lastUpdated = QDateTime::fromSecsSinceEpoch(fileDetail.value("time_updated").toInt());

        return result;
}

QFuture<SteamModInfo> ModDownloader::fetchModInfo(const QString &id)
{
    QNetworkRequest request(getPublishedFileDetails);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QString postData;
    postData.append("itemcount=1&");
    postData.append("publishedfileids[0]=").append(id);

    QNetworkReply *reply = qnam_.post(request, postData.toUtf8());
    return QtFuture::connect(reply, &QNetworkReply::finished)
            .then([reply]
    {
        reply->deleteLater();
        return reply->readAll();
    }).then(QtFuture::Launch::Async, parseSteamModInfo);
}

QFuture<QString> ModDownloader::downloadModVersion(const SteamModInfo &info)
{
    QNetworkRequest request(info.downloadUrl);

    QTemporaryFile *zipFile = new QTemporaryFile(this);
    assert(zipFile->open());

    QNetworkReply *reply = qnam_.get(request);
    connect(reply, &QIODevice::readyRead, [reply, zipFile]
    {
        zipFile->write(reply->readAll());
    });

    return QtFuture::connect(reply, &QNetworkReply::finished)
            .then([reply, zipFile]
    {
        zipFile->write(reply->readAll());
        reply->deleteLater();
    }).then(QtFuture::Launch::Async, [this, info, zipFile]
    {
        // Folder Structure: {cachePath}/workshop-{id}/{lastUpdated}/
        // lastUpdated resembles ISO8601, but uses dashes so that folder names are valid on Windows.
        QDir cacheDir(config_.cachePath());
        QDir modDir(cacheDir.absoluteFilePath(QString("workshop-%1").arg(info.id)));
        QDir modVersionDir(modDir.absoluteFilePath(info.lastUpdated.toString("yyyy-MM-ddTHH-mm-ss")));

        zipFile->seek(0);
        JlCompress::extractDir(zipFile, modVersionDir.absolutePath());
        zipFile->deleteLater();

        return modVersionDir.absolutePath();
    });
}

}  // namespace iimodmanager
