#ifndef MODINFO_H
#define MODINFO_H

#include <QIODevice>
#include <QString>
#include "iimodman-lib_global.h"
#include "modmanconfig.h"


namespace iimodmanager {

enum IIMODMANLIBSHARED_EXPORT ModLocation {
    CACHED,
    INSTALLED
};

//! The contents of a mod's modinfo.txt.
class IIMODMANLIBSHARED_EXPORT ModInfo
{
public:
    ModInfo();
    ModInfo(const QString &id, const QString &name = QString());

    inline const QString &id() const { return id_; }
    inline const QString &name() const { return name_; }
    inline const QString &version() const { return version_; }

    inline bool isSteam() const { return id_.startsWith("workshop-"); }
    QString steamId() const { return id_.sliced(9); }

    inline const QString toString() const { return QStringLiteral("%1 [%2]").arg(name_, id_); }

    static const ModInfo readModInfo(const ModManConfig &config, const QString &id, ModLocation location);
    static const ModInfo readModInfo(QIODevice &file, const QString &id);

private:
    QString id_;
    QString name_;
    QString version_;
};

}  // namespace iimodmanager

#endif // MODINFO_H
