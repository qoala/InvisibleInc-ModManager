#ifndef MOD_H
#define MOD_H

#include <QIODevice>
#include <QString>
#include "iimodman-lib_global.h"
#include "modmanconfig.h"


namespace iimodmanager {

enum IIMODMANLIBSHARED_EXPORT ModLocation {
    CACHED,
    INSTALLED
};

class IIMODMANLIBSHARED_EXPORT Mod
{
public:
    Mod(const QString &id, ModLocation location = CACHED, const QString &name = QString());

    inline const QString &id() const { return id_; }
    inline ModLocation location() const { return location_; }
    inline const QString &name() const { return name_; }

    inline bool isSteam() const { return id_.startsWith("workshop-"); }
    inline const QString toString() const { return QStringLiteral("%1 [%2]").arg(name_, id_); }

    static const Mod readModInfo(const ModManConfig &config, const QString &id, ModLocation location = CACHED);
    static const Mod readModInfo(QIODevice &file, const QString &id, ModLocation location = CACHED);

private:
    const QString id_;
    const ModLocation location_;
    QString name_;
};

}  // namespace iimodmanager

#endif // MOD_H
