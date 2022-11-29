#ifndef MODINFO_H
#define MODINFO_H

#include "iimodman-lib_global.h"

#include <QString>
#include <memory>

class QIODevice;


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

    enum IDStatus {
        // Given ID is always used as-is.
        ID_LOCKED,
        // Given non-steam IDs may be replaced if the mod-info specifies a steam workshop ID.
        ID_TENTATIVE,
    };
    static const ModInfo readModInfo(QIODevice &file, const QString &id, IDStatus status = ID_LOCKED);

private:
    class Impl;

    std::shared_ptr<const Impl> impl;
};

}  // namespace iimodmanager

#endif // MODINFO_H
