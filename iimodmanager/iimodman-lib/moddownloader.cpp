#include "moddownloader.h"

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

SteamModInfo parseSteamModInfo(const QByteArray rawData)
{
        QJsonParseError errors;
        const QJsonDocument json = QJsonDocument::fromJson(rawData, &errors);

        if (json.isNull())
        {
            qDebug() << rawData;
            qFatal("JSON Parse Error: %s", errors.errorString().toUtf8().constData());
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

ModInfoCall::ModInfoCall(const ModManConfig &config, QNetworkAccessManager &qnam, QObject *parent)
    : QObject(parent), config_(config), qnam_(qnam)
{}

void ModInfoCall::start(const QString &id)
{
    QString callDebugInfo = QString("ModInfo(%1)").arg(id);

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
        result_ = parseSteamModInfo(reply->readAll());
        reply->deleteLater();
        emit finished();
    });
}

}  // namespace iimodmanager
