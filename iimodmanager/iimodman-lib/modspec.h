#ifndef IIMODMANAGER_MODSPEC_H
#define IIMODMANAGER_MODSPEC_H

#include "iimodman-lib_global.h"

#include <experimental/propagate_const>

#include <QString>
#include <memory>

class QIODevice;
template <typename T> class QList;

namespace iimodmanager {

class SpecMod;

//! List of mods in a mod-specification. A current or prospective state for installed mods.
class IIMODMANLIBSHARED_EXPORT ModSpec
{

public:
    ModSpec();
    ModSpec(const QList<SpecMod> &mods);

    const QList<SpecMod> mods() const;
    bool contains(QString modId) const;

    void reserve(qsizetype);
    void append(const SpecMod &specMod);

    bool appendFromFile(QIODevice &file, const QString &debugRef = QString());

    ~ModSpec();
private:
    class Impl;
    std::experimental::propagate_const<std::unique_ptr<Impl>> impl;
};

class IIMODMANLIBSHARED_EXPORT SpecMod
{
public:
    SpecMod(const QString &id, const QString &name);
    SpecMod(const QString &id, const QString &versionId, const QString &name, const QString &versionName);

    static std::optional<SpecMod> fromSpecString(const QString &line, const QString &debugRef = QString(), qsizetype debugLine = -1);

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

} // namespace iimodmanager

#endif // IIMODMANAGER_MODSPEC_H
