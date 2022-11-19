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

const QString EMPTY_VERSION = QStringLiteral("-");

const QString &displayVersion(const QString &version)
{
    return version.isNull() ? EMPTY_VERSION : version;
}

bool isSteamModId(const QString &input)
{
    QRegularExpression re(QStringLiteral("^workshop-\\d+$"));
    return re.match(input).hasMatch();
}
QString parseSteamModUrl(const QString &input)
{
    QUrl url(input);
    if (url.host() != "steamcommunity.com")
        return QString();

    QRegularExpression re(QStringLiteral("(^|&)id=(\\d+)(&|$)"));
    auto match = re.match(url.query());
    if (match.hasMatch())
        return QStringLiteral("workshop-%1").arg(match.captured(2));
    return QString();
}

} // namespace util

} // namespace iimodmanager
