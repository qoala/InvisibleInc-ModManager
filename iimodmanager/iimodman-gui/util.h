#include <QString>

namespace iimodmanager {

class CachedMod;
class InstalledMod;


namespace util {

    bool compareCachedModNames(const CachedMod &a, const CachedMod &b);
    bool compareModNames(const InstalledMod &a, const InstalledMod &b);
    const QString &displayVersion(const QString &version);

} // namespace util

} // namespace iimodmanager
