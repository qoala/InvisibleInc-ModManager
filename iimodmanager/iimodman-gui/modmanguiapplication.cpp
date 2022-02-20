#include "modmanguiapplication.h"

#include <modcache.h>
#include <moddownloader.h>
#include <modlist.h>

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
    modDownloader_ = new ModDownloader(config_, this);
}

}  // namespace iimodmanager
