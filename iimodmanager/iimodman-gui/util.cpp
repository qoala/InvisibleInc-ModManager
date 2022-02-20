#include "util.h"

#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

namespace util {

bool compareModNames(const InstalledMod &a, const InstalledMod &b)
{
    return a.info().name() < b.info().name();
}

} // namespace util

} // namespace iimodmanager
