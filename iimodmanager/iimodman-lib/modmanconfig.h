#ifndef MODMANCONFIG_H
#define MODMANCONFIG_H

#include <QSettings>
#include <QString>

namespace iimodmanager {

class ModManConfig
{
public:
    static const QString applicationName;
    static const QString organizationName;

    ModManConfig();

    const QString downloadPath() const;
    void setDownloadPath(const QString&);
    const QString installPath() const;
    void setInstallPath(const QString&);
private:
    QSettings settings_;

    static const QString defaultDownloadPath();
    static const QString defaultInstallPath();
};

}  // namespace iimodmanager

#endif // MODMANCONFIG_H
