#include "modmanguiapplication.h"

namespace iimodmanager {

ModManGuiApplication::ModManGuiApplication(int &argc, char **argv[])
    : QApplication(argc, *argv), config_()
{
    setApplicationName(ModManConfig::applicationName);
    setOrganizationName(ModManConfig::organizationName);

    cache_ = new ModCache(config_, this);
    modList_ = new ModList(config_, cache_, this);
    cache_->refresh(ModCache::LATEST_ONLY);
    modList_->refresh();
}

}  // namespace iimodmanager
