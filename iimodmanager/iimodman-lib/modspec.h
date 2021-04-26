#ifndef IIMODMANAGER_MODSPEC_H
#define IIMODMANAGER_MODSPEC_H

#include "iimodman-lib_global.h"

#include <experimental/propagate_const>

#include <QString>
#include <memory>

template <typename T> class QList;

namespace iimodmanager {

class SpecMod;

//! List of mods in a mod-specification. A current or prospective state for installed mods.
class IIMODMANLIBSHARED_EXPORT ModSpec
{
public:
    ModSpec(const QList<SpecMod> &mods);

    QList<SpecMod> mods();
    const QList<SpecMod> mods() const;

private:
    class Impl;
    std::experimental::propagate_const<std::unique_ptr<Impl>> impl;
};

class SpecMod
{
public:
    SpecMod(const QString &id, const QString &name);
    SpecMod(const QString &id, const QString &versionId, const QString &name, const QString &versionName);

    static SpecMod forModId(const QString &id);
    static SpecMod forModVersionId(const QString &id, const QString &versionId);

    QString id() const;
    QString versionId() const;
    QString name() const;
    QString versionName() const;

    QString asSpecString() const;
    QString asVersionedSpecString() const;

private:
    class Impl;
    std::shared_ptr<const Impl> impl;
};

inline SpecMod SpecMod::forModId(const QString &id)
{
    return SpecMod(id, QString());
}

inline SpecMod SpecMod::forModVersionId(const QString &id, const QString &versionId)
{
    return SpecMod(id, versionId, QString(), QString());
}

} // namespace iimodmanager

#endif // IIMODMANAGER_MODSPEC_H
