#ifndef MODMANGUIAPPLICATION_H
#define MODMANGUIAPPLICATION_H

#include <QApplication>
#include <modmanconfig.h>
#include <modcache.h>
#include <modlist.h>

namespace iimodmanager {

class ModDownloader;

class ModManGuiApplication: public QApplication
{
public:
    ModManGuiApplication(int &argc, char **argv[]);

    inline ModManConfig &config() { return config_; }
    inline ModCache &cache() { return *cache_; }
    inline ModList &modList() { return *modList_; }
    inline ModDownloader &modDownloader() { return *modDownloader_; }

    //! Refresh cache and mod list at the GUI's default level.
    //! (LATEST_ONLY for cache, FULL for installed)
    //!
    //! Commands should call this immediately before making changes,
    //! in case of external changes.
    void refreshMods();

private:
    ModManConfig config_;
    ModCache *cache_;
    ModList *modList_;
    ModDownloader *modDownloader_;
};

}  // namespace iimodmanager

#endif // MODMANGUIAPPLICATION_H

