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
    Mod();
    Mod(const QString &id);

    inline const QString &id() const { return id_; }
    inline const QString &name() const { return name_; }
    inline const QString &version() const { return version_; }

    inline bool isSteam() const { return id_.startsWith("workshop-"); }
    inline const QString toString() const { return QStringLiteral("%1 [%2]").arg(name_, id_); }

    static const Mod readModInfo(const ModManConfig &config, const QString &id, ModLocation location);
    static const Mod readModInfo(QIODevice &file, const QString &id);

private:
    QString id_;
    QString name_;
    QString version_;
};

}  // namespace iimodmanager

#endif // MOD_H
