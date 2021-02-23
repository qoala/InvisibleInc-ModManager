#ifndef MODLIST_H
#define MODLIST_H

#include "iimodman-lib_global.h"
#include "mod.h"
#include "modmanconfig.h"

namespace iimodmanager {

class IIMODMANLIBSHARED_EXPORT ModList
{
public:
    ModList(const std::list<Mod> &mods, ModLocation location = CACHED);

    inline ModLocation location() const { return location_; }
    const std::list<Mod> list() const { return mods_; }

    static ModList readCurrent(const ModManConfig &config, ModLocation location = CACHED);
private:
    const ModLocation location_;
    const std::list<Mod> mods_;
};

}  // namespace iimodmanager

#endif // MODLIST_H
