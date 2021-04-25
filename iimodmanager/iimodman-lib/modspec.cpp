#include "modspec.h"

#include <QList>
#include <QString>

namespace iimodmanager {

// {cacheModId}:{installedModId}:{cacheVersionId}:{unused}:{free text}
// free text: human readable text with the mod name (and version specifier if versioned and available)
// unused: In the future, could add a single-line json string or other format for additional fields.
const QString specLineFormat = QStringLiteral("%1::%2::%3");

class ModSpec::Impl
{
public:
    Impl(const QList<SpecMod> &mods);

    QList<SpecMod> mods;
};

class SpecMod::Impl
{
public:
    Impl(const QString &id, const QString &versionId, const QString &name, const QString &versionName);

    const QString id;
    const QString versionId;
    const QString name;
    const QString versionName;

    QString asSpecString() const;
    QString asVersionedSpecString() const;
};

ModSpec::ModSpec(const QList<SpecMod> &mods)
    : impl{std::make_unique<Impl>(mods)}
{}

QList<SpecMod> ModSpec::mods()
{
    return impl->mods;
}

const QList<SpecMod> ModSpec::mods() const
{
    return impl->mods;
}

ModSpec::Impl::Impl(const QList<SpecMod> &mods)
    : mods(mods)
{}

SpecMod::SpecMod(const QString &id, const QString &name)
    : SpecMod(id, QString(), name, QString())
{}

SpecMod::SpecMod(const QString &id, const QString &versionId, const QString &name, const QString &versionName)
    : impl{std::make_shared<Impl>(id, versionId, name, versionName)}
{}

QString SpecMod::id() const
{
    return impl->id;
}

QString SpecMod::versionId() const
{
    return impl->versionId;
}

QString SpecMod::name() const
{
    return impl->name;
}

QString SpecMod::versionName() const
{
    return impl->versionName;
}

QString SpecMod::asSpecString() const
{
    return impl->asSpecString();
}

QString SpecMod::asVersionedSpecString() const
{
    return impl->asVersionedSpecString();
}

SpecMod::Impl::Impl(const QString &id, const QString &versionId, const QString &name, const QString &versionName)
    : id(id), versionId(versionId), name(name), versionName(versionName)
{}

QString SpecMod::Impl::asSpecString() const
{
    return specLineFormat.arg(id, QString(), name);
}

QString SpecMod::Impl::asVersionedSpecString() const
{
    const QString desc = versionName.isEmpty() ? name : QStringLiteral("%1 [%2]").arg(name, versionName);
    return specLineFormat.arg(id, versionId, desc);
}

} // namespace iimodmanager
