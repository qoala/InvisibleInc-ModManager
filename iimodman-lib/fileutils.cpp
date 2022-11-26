#include "fileutils.h"

#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QString>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(fileutils)
Q_LOGGING_CATEGORY(fileutils, "files", QtWarningMsg);

bool FileUtils::removeModDir(const QString &path, QString *errorInfo)
{
    QDir dir(path);
    if (dir.exists() && !dir.isEmpty())
    {
        if (dir.exists("modinfo.txt"))
        {
            qCDebug(fileutils).noquote() << "Deleting existing" << path;
            dir.removeRecursively();
        }
        else
        {
            QString msg = QStringLiteral("Output directory non-empty and not a mod folder: %1").arg(path.toUtf8().constData());
            qCCritical(fileutils).noquote() << msg;
            if (errorInfo)
                *errorInfo = msg;
            return false;
        }
    }
    return true;
}

bool FileUtils::copyRecursively(const QString &srcPath, const QString &destPath, QString *errorInfo)
{
    QDir srcDir(srcPath);
    QDir destDir(destPath);
    if (!srcDir.exists())
    {
        if (errorInfo)
            *errorInfo = QStringLiteral("Missing source dir: %1").arg(srcPath);
        return false;
    }
    if (!destDir.mkpath(destPath))
    {
        if (errorInfo)
            *errorInfo = QStringLiteral("Failed to create destination dir: %1").arg(destPath);
        return false;
    }

    for(auto entry : srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden))
    {
        QString path = destDir.filePath(entry);
        if (!copyRecursively(srcDir.filePath(entry), path, errorInfo))
            return false;
    }
    for(auto entry : srcDir.entryList(QDir::Files | QDir::Hidden))
    {
        if (!QFile::copy(srcDir.filePath(entry), destDir.filePath(entry)))
        {
            if (errorInfo)
                *errorInfo = QStringLiteral("Failed to copy mod file: %1 to %2/").arg(srcDir.filePath(entry), destPath);
            return false;
        }
    }

    return true;
}

const QJsonObject FileUtils::readJSON(const QString &filePath, QString *errorInfo)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorInfo)
            *errorInfo = "Failed to open json file.";
        return QJsonObject();
    }
    QByteArray rawData = file.readAll();
    file.close();

    QJsonParseError errors;
    const QJsonDocument json = QJsonDocument::fromJson(rawData, &errors);

    if (json.isNull())
    {
        if (errorInfo)
            *errorInfo = errors.errorString();
        qCWarning(fileutils) << "JSON Parse Error:", errors.errorString();
        qCDebug(fileutils) << rawData;
        return QJsonObject();
    }
    if (!json.isObject())
    {
        if (errorInfo)
            *errorInfo = "Not a JSON object.";
        qCWarning(fileutils) << "JSON: not an object";
        return QJsonObject();
    }

    return json.object();
}

bool FileUtils::writeJSON(const QString &filePath, const QJsonObject &root, QString *errorInfo)
{
    QJsonDocument json(root);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(json.toJson());
        return true;
    }

    if (errorInfo)
        *errorInfo = "Failed to open json file for writing.";
    return false;
}

} // namespace iimodmanager
