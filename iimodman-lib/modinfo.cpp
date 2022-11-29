#include "modinfo.h"

#include <QDir>
#include <QGlobalStatic>
#include <QIODevice>
#include <QRegularExpression>
#include <QSettings>
#include <QString>
#include <QTextStream>

namespace iimodmanager {

//! Private implementation of ModInfo.
class ModInfo::Impl
{
public:
    Impl(const QString &id, const QString &name, const QString &version);

    inline const QString &id() const { return id_; }
    inline const QString &name() const { return name_; }
    inline const QString &version() const { return version_; }

    static const std::shared_ptr<const Impl> emptyImpl;

private:
    const QString id_;
    const QString name_;
    const QString version_;
};

//! Dynamically initialized singleton empty ModInfo.
const std::shared_ptr<const ModInfo::Impl> ModInfo::Impl::emptyImpl = std::make_shared<ModInfo::Impl>(QString(), QString(), QString());

ModInfo::ModInfo()
    : impl(Impl::emptyImpl)
{}

ModInfo::ModInfo(const QString &id, const QString &name)
    : impl{std::make_shared<Impl>(id, name, QString())}
{
    if (id.isEmpty())
        clear();
}

const QString &ModInfo::id() const
{
    return impl->id();
}

const QString &ModInfo::name() const
{
    return impl->name();
}

const QString &ModInfo::version() const
{
    return impl->version();
}

static bool isSteamId(const QString &id)
{
    static const QRegularExpression workshopRe("^workshop-\\d+$");
    return id.contains(workshopRe);
}

bool ModInfo::isSteam() const
{
    return isSteamId(impl->id());
}

QString ModInfo::steamId() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return impl->id().sliced(9);
#else
    return impl->id().mid(9);
#endif
}

bool ModInfo::isEmpty() const
{
    return Impl::emptyImpl == impl;
}

QString ModInfo::toString() const
{
    return QStringLiteral("%1 [%2]").arg(impl->name(), impl->id());
}

void ModInfo::clear()
{
    impl = Impl::emptyImpl;
}

const ModInfo ModInfo::readModInfo(QIODevice &file, const QString &id, IDStatus status)
{
    if (id.isEmpty())
        return ModInfo();

    QString steamId;
    QString name;
    QString version;

    if (file.isOpen() || file.open(QIODevice::ReadOnly))
    {
        static const QRegularExpression linePattern("^\\s*(\\w+)\\s*=\\s*(.*)$");
        static const QRegularExpression digitPattern("^\\d+$");
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QRegularExpressionMatch match = linePattern.match(line);
            if (match.hasMatch())
            {
                QString key = match.captured(1);
                if (key == "name")
                {
                    name = match.captured(2).trimmed();
                }
                else if (key == "version")
                {
                    version = match.captured(2).trimmed();
                }
                else if (key == "workshop")
                {
                    steamId = match.captured(2).trimmed();
                    if (!steamId.contains(digitPattern))
                        steamId.clear();
                }
            }
        }
    }

    ModInfo mod;
    const QString modId = (status == ID_LOCKED || isSteamId(id) || steamId.isEmpty()) ? id : QStringLiteral("workshop-%1").arg(steamId);
    if (!version.isEmpty() && !version.startsWith('v'))
        version = 'v' + version;
    mod.impl = std::make_shared<Impl>(modId, name, version.isEmpty() ? QString() : version);
    return mod;
}

ModInfo::Impl::Impl(const QString &id, const QString &name, const QString &version)
    : id_(id), name_(name), version_(version)
{}

}  // namespace iimodmanager
