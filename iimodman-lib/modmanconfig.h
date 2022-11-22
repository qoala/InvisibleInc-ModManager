#ifndef MODMANCONFIG_H
#define MODMANCONFIG_H

#include "iimodman-lib_global.h"
#include <QSettings>


namespace iimodmanager {

class IIMODMANLIBSHARED_EXPORT ModManConfig
{
public:
    static const QString applicationName;
    static const QString organizationName;

    ModManConfig();

    // Configurable paths
    const QString cachePath() const;
    void setCachePath(const QString&);
    const QString installPath() const;
    void setInstallPath(const QString&);
    const QString localPath() const;
    void setLocalPath(const QString&);

    // Derived paths
    const QString modPath() const;
    const QString savePath() const;

    // Defaults
    const QString defaultCachePath() const;
    const QString defaultInstallPath() const;
    const QString defaultLocalPath() const;

private:
    QSettings settings_;
};

}  // namespace iimodmanager

#endif // MODMANCONFIG_H
