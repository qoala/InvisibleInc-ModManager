#include "modmanconfig.h"

#include <QString>

namespace iimodmanager {

const QString ModManConfig::applicationName = QStringLiteral("InvisibleInc Mod Manager");
const QString ModManConfig::organizationName = QStringLiteral("QoalatyEngineering");

const QString kDownloadPath = QStringLiteral("downloadPath");
const QString kInstallPath = QStringLiteral("installPath");

ModManConfig::ModManConfig()
    : settings_(organizationName, applicationName)
{}

const QString ModManConfig::downloadPath() const
{
    return this->settings_.value(kDownloadPath, this->defaultDownloadPath()).toString();
}

void ModManConfig::setDownloadPath(const QString& value)
{
    this->settings_.setValue(kDownloadPath, value);
}

const QString ModManConfig::installPath() const
{
    return this->settings_.value(kInstallPath, this->defaultInstallPath()).toString();
}

void ModManConfig::setInstallPath(const QString& value)
{
    this->settings_.setValue(kInstallPath, value);
}

const QString ModManConfig::defaultDownloadPath()
{
    // Linux default.
    return QStringLiteral("~/.local/share/Steam/steamapps/common/InvisibleInc/mods-dl");
}

const QString ModManConfig::defaultInstallPath()
{
    // Linux default.
    return QStringLiteral("~/.local/share/Steam/steamapps/common/InvisibleInc/mods");
    // TODO: Detect platform and define other reasonable defaults.
}

}  // namespace iimodmanager
