#include "modmanconfig.h"

#include <QDir>
#include <QString>

namespace iimodmanager {

const QString ModManConfig::applicationName = QStringLiteral("InvisibleInc Mod Manager");
const QString ModManConfig::organizationName = QStringLiteral("QoalatyEngineering");

const QString cachePathKey = QStringLiteral("core/cachePath");
const QString installPathKey = QStringLiteral("core/installPath");
const QString localPathKey = QStringLiteral("core/localPath");

ModManConfig::ModManConfig()
#ifdef Q_OS_WIN
    : settings_(QSettings::IniFormat, QSettings::UserScope, organizationName, applicationName)
#else
    : settings_(QSettings::NativeFormat, QSettings::UserScope, organizationName, applicationName)
#endif
{}

const QString ModManConfig::cachePath() const
{
    return this->settings_.value(cachePathKey, this->defaultCachePath()).toString();
}

void ModManConfig::setCachePath(const QString &value)
{
    this->settings_.setValue(cachePathKey, value);
}

const QString ModManConfig::installPath() const
{
    return this->settings_.value(installPathKey, this->defaultInstallPath()).toString();
}

void ModManConfig::setInstallPath(const QString &value)
{
    this->settings_.setValue(installPathKey, value);
}

const QString ModManConfig::localPath() const
{
    return this->settings_.value(localPathKey, this->defaultLocalPath()).toString();
}

void ModManConfig::setLocalPath(const QString &value)
{
    this->settings_.setValue(localPathKey, value);
}

const QString ModManConfig::modPath() const
{
    return installPath() + "/mods";
}

const QString ModManConfig::savePath() const
{
    return localPath() + "/saves";
}

const QString ModManConfig::defaultCachePath() const
{
    return installPath() + "/mods-cache";
}

const QString ModManConfig::defaultLocalPath() const
{
#ifdef Q_OS_LINUX
    return QDir::homePath() +  "/.local/share/Klei/InvisibleInc";
#elif defined Q_OS_WIN
    return QDir::homePath() + "/Documents/Klei/InvisibleInc";
#elif defined Q_OS_MAC
    return QString();
#else
    return QString();
#endif
}

const QString ModManConfig::defaultInstallPath() const
{
#ifdef Q_OS_LINUX
    return QDir::homePath() +  "/.local/share/Steam/steamapps/common/InvisibleInc";
#elif defined Q_OS_WIN
    return "C:/Program Files (x86)/Steam/steamapps/common/InvisibleInc";
#elif defined Q_OS_MAC
    return QString();
#else
    return QString();
#endif
}

}  // namespace iimodmanager
