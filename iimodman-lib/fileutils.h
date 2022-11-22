#ifndef IIMODMANAGER_FILEUTILS_H
#define IIMODMANAGER_FILEUTILS_H

#include "iimodman-lib_global.h"


namespace iimodmanager {

class FileUtils
{
public:
    static bool removeModDir(const QString &path, QString *errorInfo = nullptr);
    static bool copyRecursively(const QString &srcPath, const QString &destPath, QString *errorInfo = nullptr);
};

} // namespace iimodmanager

#endif // IIMODMANAGER_FILEUTILS_H
