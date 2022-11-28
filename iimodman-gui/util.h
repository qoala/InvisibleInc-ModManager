#ifndef IIMODMAN_UTIL_H
#define IIMODMAN_UTIL_H
#include <QString>

namespace iimodmanager {

class CachedMod;
class InstalledMod;
class ModInfo;
class SpecMod;


namespace util {

    bool compareCachedModNames(const CachedMod &a, const CachedMod &b);
    bool compareInstalledModNames(const InstalledMod &a, const InstalledMod &b);
    bool compareSpecModNames(const SpecMod &a, const SpecMod &b);

    const QString &displayVersion(const QString &version);
    const QString displayInfo(const ModInfo &info, const QString &alias = QString());
    const QString displayInfo(const SpecMod &sm);

    //! True if the input is a steam mod ID.
    bool isSteamModId(const QString &inputId);
    //! Returns a steam mod ID, if the string parses correctly, otherwise the default QString.
    QString parseSteamModUrl(const QString &input);
    //! Returns the steam workshop ID from a mod ID. Assumes that the mod ID already satisfies #isSteamModId.
    QString toSteamId(const QString &modId);
    //! Returns a mod ID from a steam workshop ID.
    QString fromSteamId(const QString &workshopId);

} // namespace util

} // namespace iimodmanager

#endif // IIMODMAN_UTIL_H
