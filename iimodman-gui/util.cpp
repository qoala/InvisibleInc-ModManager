#include "util.h"

#include <QRegularExpression>
#include <QUrl>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

namespace util {

bool compareCachedModNames(const CachedMod &a, const CachedMod &b)
{
    return a.info().name() < b.info().name();
}

bool compareInstalledModNames(const InstalledMod &a, const InstalledMod &b)
{
    return a.info().name() < b.info().name();
}

bool compareSpecModNames(const SpecMod &a, const SpecMod &b)
{
    return a.name() < b.name();
}


const QString &displayVersion(const QString &version)
{
    static const QString emptyVersion = QStringLiteral("-");
    return version.isNull() ? emptyVersion : version;
}

const QString displayInfo(const ModInfo &info, const QString &alias)
{
    if (alias.isEmpty())
        return QStringLiteral("%2 [%1]").arg(info.id(), info.name());
    else
        return QStringLiteral("%2 [%1@%3]").arg(info.id(), info.name(), alias);
}

const QString displayInfo(const SpecMod &sm)
{
    if (sm.alias().isEmpty())
        return QStringLiteral("%2 [%1]").arg(sm.id(), sm.name());
    else
        return QStringLiteral("%2 [%1@%3]").arg(sm.id(), sm.name(), sm.alias());
}

bool isSteamModId(const QString &input)
{
    static const QRegularExpression re(QStringLiteral("^workshop-\\d+$"));
    return input.contains(re);
}
QString parseSteamModUrl(const QString &input)
{
    QUrl url(input);
    if (url.host() != "steamcommunity.com")
        return QString();

    static const QRegularExpression re(QStringLiteral("(^|&)id=(\\d+)(&|$)"));
    auto match = re.match(url.query());
    if (match.hasMatch())
        return QStringLiteral("workshop-%1").arg(match.captured(2));
    return QString();
}
QString toSteamId(const QString &modId)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return modId.sliced(9);
#else
    return modId.mid(9);
#endif
}
QString fromSteamId(const QString &workshopId)
{
    return QStringLiteral("workshop-%1").arg(workshopId);
}

} // namespace util

} // namespace iimodmanager
