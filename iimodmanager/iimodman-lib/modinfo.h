#ifndef MODINFO_H
#define MODINFO_H

#include <QIODevice>
#include <QString>
#include "iimodman-lib_global.h"
#include "modmanconfig.h"


namespace iimodmanager {

//! The contents of a mod's modinfo.txt.
//! Immutable with implicitly shared storage on copy.
class IIMODMANLIBSHARED_EXPORT ModInfo
{
public:
    ModInfo();
    ModInfo(const QString &id, const QString &name = QString());

    const QString &id() const;
    const QString &name() const;
    const QString &version() const;

    bool isSteam() const;
    QString steamId() const;

    bool isEmpty() const;
    QString toString() const;

    //! Clear the contents and make this ModInfo empty.
    void clear();

    static const ModInfo readModInfo(QIODevice &file, const QString &id);

private:
    class Impl;

    std::shared_ptr<const Impl> impl;
};

}  // namespace iimodmanager

#endif // MODINFO_H
