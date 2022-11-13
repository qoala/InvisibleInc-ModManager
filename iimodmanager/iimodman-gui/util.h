#ifndef IIMODMAN_UTIL_H
#define IIMODMAN_UTIL_H
#include <QString>

namespace iimodmanager {

class CachedMod;
class InstalledMod;
class SpecMod;


namespace util {

    bool compareCachedModNames(const CachedMod &a, const CachedMod &b);
    bool compareInstalledModNames(const InstalledMod &a, const InstalledMod &b);
    bool compareSpecModNames(const SpecMod &a, const SpecMod &b);
    const QString &displayVersion(const QString &version);

} // namespace util

} // namespace iimodmanager

#endif // IIMODMAN_UTIL_H
