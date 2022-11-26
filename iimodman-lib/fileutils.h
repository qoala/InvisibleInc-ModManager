#ifndef IIMODMANAGER_FILEUTILS_H
#define IIMODMANAGER_FILEUTILS_H

#include "iimodman-lib_global.h"

class QJsonObject;


namespace iimodmanager {

namespace FileUtils
{
    bool removeModDir(const QString &path, QString *errorInfo = nullptr);
    bool copyRecursively(const QString &srcPath, const QString &destPath, QString *errorInfo = nullptr);

    const QJsonObject readJSON(const QString &filePath, QString *errorInfo = nullptr);
    bool writeJSON(const QString &filePath, const QJsonObject &root, QString *errorInfo = nullptr);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_FILEUTILS_H
