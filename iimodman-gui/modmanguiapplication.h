#ifndef MODMANGUIAPPLICATION_H
#define MODMANGUIAPPLICATION_H

#include "guiconfig.h"

#include <QApplication>

namespace iimodmanager {

class ModCache;
class ModList;
class ModDownloader;

class ModManGuiApplication: public QApplication
{
public:
    ModManGuiApplication(int &argc, char **argv[]);

    inline const GuiConfig &config() const { return config_; }
    inline GuiConfig &mutableConfig() { return config_; }
    inline const ModCache &cache() const { return *cache_; }
    inline ModCache &cache() { return *cache_; }
    inline const ModList &modList() const { return *modList_; }
    inline ModList &modList() { return *modList_; }
    inline ModDownloader &modDownloader() { return *modDownloader_; }

    //! Refresh cache and mod list at the GUI's default level.
    //! (LATEST_ONLY for cache, FULL for installed)
    //!
    //! Commands should call this immediately before making changes,
    //! in case of external changes.
    void refreshMods();

private:
    GuiConfig config_;
    ModCache *cache_;
    ModList *modList_;
    ModDownloader *modDownloader_;
};

}  // namespace iimodmanager

#endif // MODMANGUIAPPLICATION_H

