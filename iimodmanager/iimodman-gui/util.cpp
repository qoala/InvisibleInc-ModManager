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

const QString EMPTY_VERSION = QStringLiteral("-");

const QString &displayVersion(const QString &version)
{
    return version.isNull() ? EMPTY_VERSION : version;
}

} // namespace util

} // namespace iimodmanager
