#include "fileutils.h"

#include <QDebug>
#include <QDir>
#include <QLoggingCategory>
#include <QString>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(fileutils)
Q_LOGGING_CATEGORY(fileutils, "files", QtWarningMsg);

bool FileUtils::removeModDir(const QString &path)
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
            qFatal("Output directory non-empty and not a mod folder: %s",
                   path.toUtf8().constData());
        }
    }
    return true;
}

bool FileUtils::copyRecursively(const QString &srcPath, const QString &destPath)
{
    QDir srcDir(srcPath);
    QDir destDir(destPath);
    if (!srcDir.exists())
        return false;
    if (!destDir.mkpath(destPath))
        return false;

    for(auto entry : srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden))
    {
        QString path = destDir.filePath(entry);
        if (!copyRecursively(srcDir.filePath(entry), path))
            return false;
    }
    for(auto entry : srcDir.entryList(QDir::Files | QDir::Hidden))
    {
        if (!QFile::copy(srcDir.filePath(entry), destDir.filePath(entry)))
            return false;
    }

    return true;
}

} // namespace iimodmanager
