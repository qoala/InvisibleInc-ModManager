#include "modspec.h"

#include <QDebug>
#include <QList>
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
    Impl(const QList<SpecMod> &mods);

    QList<SpecMod> mods;

    bool readFromFile(QIODevice &file);
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

bool ModSpec::readFromFile(QIODevice &file)
{
    return impl->readFromFile(file);
}

ModSpec::Impl::Impl(const QList<SpecMod> &mods)
    : mods(mods)
{}

bool ModSpec::Impl::readFromFile(QIODevice &file)
{
    if (file.isOpen() || file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        mods.clear();
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine().simplified();
            // skip blank lines and comment lines
            if (line.isEmpty() || line.startsWith('#'))
                continue;

            std::optional<SpecMod> mod = SpecMod::fromSpecString(line);
            if (mod)
                mods.append(*mod);
        }
        return true;
    }
    return false;
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
