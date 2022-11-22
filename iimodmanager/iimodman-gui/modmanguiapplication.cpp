#include "modmanguiapplication.h"

#include <QIcon>
#include <modcache.h>
#include <moddownloader.h>
#include <modlist.h>

namespace iimodmanager {

ModManGuiApplication::ModManGuiApplication(int &argc, char **argv[])
    : QApplication(argc, *argv), config_()
{
    setApplicationName(ModManConfig::applicationName);
    setOrganizationName(ModManConfig::organizationName);
    setWindowIcon(QIcon(":icons/iimm-icon-64.png"));

    cache_ = new ModCache(config_, this);
    modList_ = new ModList(config_, cache_, this);
    modDownloader_ = new ModDownloader(config_, this);

    refreshMods();
}

void ModManGuiApplication::refreshMods()
{
    cache_->refresh(ModCache::LATEST_ONLY);
    modList_->refresh();
}

}  // namespace iimodmanager
