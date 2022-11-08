#include "fileutils.h"

#include <QDebug>
#include <QDir>
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

} // namespace iimodmanager
