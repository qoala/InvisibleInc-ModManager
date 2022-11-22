#include "modspec.h"

#include <QDebug>
#include <QList>
#include <QIODevice>
#include <QMap>
#include <QString>
#include <stdexcept>

namespace iimodmanager {

// {cacheModId}:{installedModId}:{cacheVersionId}:{unused}:{free text}
// free text: human readable text with the mod name (and version specifier if versioned and available)
// unused: In the future, could add a single-line json string or other format for additional fields.
const QString specLineFormat = QStringLiteral("%1::%2::%3");
const qsizetype specLineModId = 0;
// unused: 1
const qsizetype specLineVersionId = 2;
// unused: 3
const qsizetype specLineNameStart = 4;
const qsizetype specLineSectionLength = 5;

class ModSpec::Impl
{
public:
    Impl();

    QList<SpecMod> mods;
    //! Index of mods by mod ID.
    QMap<QString, qsizetype> modIds;

    bool appendFromFile(QTextStream &in, const QString &debugRef);
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

ModSpec::ModSpec(QObject *parent)
    : QObject(parent), impl{std::make_unique<Impl>()}
{}

const QList<SpecMod> ModSpec::mods() const
{
    return impl->mods;
}

bool ModSpec::contains(QString modId) const
{
    return impl->modIds.contains(modId);
}

void ModSpec::clear()
{
    impl->mods.clear();
}

void ModSpec::reserve(qsizetype size)
{
    impl->mods.reserve(size);
}

void ModSpec::append(const SpecMod &specMod)
{
    impl->modIds[specMod.id()] = impl->mods.size();
    impl->mods.append(specMod);
}

bool ModSpec::appendFromFile(QIODevice &file, const QString &debugRef)
{
    if (file.isOpen() || file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&file);
        return impl->appendFromFile(in, debugRef);
    }
    return false;
}

bool ModSpec::appendFromFile(const QByteArray &content, const QString &debugRef)
{
    QTextStream in(content);
    return impl->appendFromFile(in, debugRef);
}

ModSpec::~ModSpec() = default;

ModSpec::Impl::Impl()
{}

bool ModSpec::Impl::appendFromFile(QTextStream &in, const QString &debugRef)
{
    qsizetype lineNo = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine().simplified();
        ++lineNo;
        // skip blank lines and comment lines
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        std::optional<SpecMod> sm = SpecMod::fromSpecString(line, debugRef, lineNo);
        if (sm)
        {
            modIds[sm->id()] = mods.size();
            mods.append(*sm);
        }
    }
    return true;
}

SpecMod::SpecMod(const QString &id, const QString &name)
    : SpecMod(id, QString(), name, QString())
{}

SpecMod::SpecMod(const QString &id, const QString &versionId, const QString &name, const QString &versionName)
    : impl{std::make_shared<Impl>(id, versionId, name, versionName)}
{}

std::optional<SpecMod> SpecMod::fromSpecString(const QString &line, const QString &debugRef, qsizetype debugLineNo)
{
    const QStringList list = line.split(':');
    if (list.length() < specLineSectionLength)
    {
        const QString ref = debugLineNo >= 0 ? QStringLiteral(" (%1:%2)").arg(debugRef, debugLineNo): QString();
        qWarning() << "Couldn't parse spec line" << ref << ": Too few separators. \"" << line << '"';
        return {};
    }

    const QString &modId = list[specLineModId];
    const QString &versionId = list[specLineVersionId];

    const QString name = line.section(':', specLineNameStart);

    return SpecMod(modId, versionId, name, QString());
}

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

SpecMod SpecMod::withoutVersion() const
{
    return SpecMod(impl->id, impl->name);
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
