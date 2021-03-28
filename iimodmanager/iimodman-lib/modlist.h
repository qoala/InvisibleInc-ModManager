#ifndef MODLIST_H
#define MODLIST_H

#include "iimodman-lib_global.h"
#include "modcache.h"
#include "modinfo.h"
#include "modmanconfig.h"

#include <QList>

namespace iimodmanager {

class InstalledMod;

//! Mods installed to the Invisible Inc application.
class IIMODMANLIBSHARED_EXPORT ModList : public QObject
{
public:
    enum RefreshLevel
    {
        //! Mod ID, content metadata, and cache version association.
        FULL,
        //! Mod ID and content metadata (modinfo.txt, modman.json).
        CONTENT_ONLY,
        //! Mod ID only.
        ID_ONLY,
    };

    ModList(const ModManConfig &config, ModCache *cache = nullptr, QObject *parent = nullptr);

    inline const ModCache *cache() const { return cache_; }
    inline const QList<InstalledMod> mods() const { return mods_; }

    inline QString modPath(const QString &modId) const { return modPath(config_, modId); };
    static QString modPath(const ModManConfig &config, const QString &modId);

    void refresh(RefreshLevel level = FULL);
private:
    const ModManConfig &config_;
    ModCache *cache_;
    QList<InstalledMod> mods_;
};

class IIMODMANLIBSHARED_EXPORT InstalledMod
{
public:
    InstalledMod(const ModList &parent, const QString &id);

    inline const QString &id() const { return id_; };
    inline const ModInfo &info() const { return info_; };

    inline bool hasCacheVersion() const { return !cacheVersionId_.isEmpty(); };
    const CachedVersion *cacheVersion() const;

    QString versionString() const;

    bool refresh(ModList::RefreshLevel level = ModList::FULL);

    InstalledMod &operator =(const InstalledMod&);

private:
    const ModList &parent;
    QString id_;
    ModInfo info_;
    QString hash_;
    QString cacheVersionId_;
};

}  // namespace iimodmanager

#endif // MODLIST_H
