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

    const QString cachePath() const;
    void setCachePath(const QString&);
    const QString installPath() const;
    void setInstallPath(const QString&);
private:
    QSettings settings_;

    static const QString defaultCachePath();
    static const QString defaultInstallPath();
};

}  // namespace iimodmanager

#endif // MODMANCONFIG_H
