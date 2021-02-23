#include "modmanconfig.h"

#include <QString>

namespace iimodmanager {

const QString ModManConfig::applicationName = QStringLiteral("InvisibleInc Mod Manager");
const QString ModManConfig::organizationName = QStringLiteral("QoalatyEngineering");

const QString cachePathKey = QStringLiteral("core/downloadPath");
const QString installPathKey = QStringLiteral("core/installPath");
const QString steamApiKeyKey = QStringLiteral("steam/apiKey");

ModManConfig::ModManConfig()
    : settings_(organizationName, applicationName)
{}

const QString ModManConfig::cachePath() const
{
    return this->settings_.value(cachePathKey, this->defaultCachePath()).toString();
}

void ModManConfig::setCachePath(const QString& value)
{
    this->settings_.setValue(cachePathKey, value);
}

const QString ModManConfig::installPath() const
{
    return this->settings_.value(installPathKey, this->defaultInstallPath()).toString();
}

void ModManConfig::setInstallPath(const QString& value)
{
    this->settings_.setValue(installPathKey, value);
}

const QString ModManConfig::steamApiKey() const
{

    return this->settings_.value(steamApiKeyKey).toString();
}

void ModManConfig::setSteamApiKey(const QString &value)
{
    this->settings_.setValue(steamApiKeyKey, value);
}

const QString ModManConfig::defaultCachePath()
{
    // Linux default.
    return QStringLiteral("~/.local/share/Steam/steamapps/common/InvisibleInc/mods-cache");
}

const QString ModManConfig::defaultInstallPath()
{
    // Linux default.
    return QStringLiteral("~/.local/share/Steam/steamapps/common/InvisibleInc/mods");
    // TODO: Detect platform and define other reasonable defaults.
}

}  // namespace iimodmanager
