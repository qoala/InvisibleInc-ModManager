#include "util.h"

#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

namespace util {

bool compareCachedModNames(const CachedMod &a, const CachedMod &b)
{
    return a.info().name() < b.info().name();
}

bool compareModNames(const InstalledMod &a, const InstalledMod &b)
{
    return a.info().name() < b.info().name();
}

} // namespace util

} // namespace iimodmanager
