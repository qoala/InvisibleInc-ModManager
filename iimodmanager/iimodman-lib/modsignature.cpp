#include "modsignature.h"

#include <QCryptographicHash>
#include <QDir>
#include <QLoggingCategory>
#include <QString>

namespace iimodmanager {

Q_DECLARE_LOGGING_CATEGORY(modsig)
Q_LOGGING_CATEGORY(modsig, "modsignature", QtWarningMsg)

void addFile(QCryptographicHash &hash, const QDir &rootDir, QString filePath)
{
    QString localPath = rootDir.relativeFilePath(filePath);
    qCDebug(modsig) << "Hashing" << localPath;
    hash.addData(localPath.toUtf8());

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        if (!hash.addData(&file))
            qCWarning(modsig) << "Failed to hash" << filePath;
    }
    else
    {
        qCWarning(modsig) << "Failed to open" << filePath;
    }
}

void addDir(QCryptographicHash &hash, const QDir &rootDir, QString dirPath)
{
    QDir dir(dirPath);

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    for (auto entry : dir.entryInfoList())
    {
        if (entry.isDir())
            addDir(hash, rootDir, entry.filePath());
        else if (entry.isFile())
            addFile(hash, rootDir, entry.filePath());
    }
}

QString ModSignature::hashModPath(const QString &dirPath)
{
    QCryptographicHash hash(QCryptographicHash::Md5);

    qCDebug(modsig) << "Begin Hashing" << dirPath;
    const QDir modDir(dirPath);
    addDir(hash, modDir, modDir.path());

    return hash.result().toHex();
}

} // namespace iimodmanager
