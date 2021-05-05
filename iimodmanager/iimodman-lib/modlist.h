#ifndef MODLIST_H
#define MODLIST_H

#include "iimodman-lib_global.h"

#include <experimental/propagate_const>

#include <QObject>
#include <memory>

template <typename T> class QList;


namespace iimodmanager {

class CachedVersion;
class ModCache;
class ModInfo;
class ModManConfig;
class SpecMod;
class SteamModInfo;


class InstalledMod;

//! Mods installed to the Invisible Inc application.
class IIMODMANLIBSHARED_EXPORT ModList : public QObject
{
public:
    //! Private implementation. Only accessible to classes in this file.
    class Impl;
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

    const QList<InstalledMod> mods() const;
    const InstalledMod *mod(const QString &id) const;

    void refresh(RefreshLevel level = FULL);
    const InstalledMod *installMod(const SpecMod &specMod);
    const InstalledMod *removeMod(const QString &modId);

    ~ModList();
private:
    std::experimental::propagate_const<std::unique_ptr<Impl>> impl;
};

class IIMODMANLIBSHARED_EXPORT InstalledMod
{
public:
    //! Private implementation. Only accessible to classes in this file.
    class Impl;

    InstalledMod(const ModList::Impl &parent, const QString &id);

    const QString &id() const;
    const ModInfo &info() const;
    const QString &hash() const;

    bool hasCacheVersion() const;
    //! The cached version matching the currently installed mod, or nullptr if there is no match.
    const CachedVersion *cacheVersion() const;

    const SpecMod asSpec() const;
    QString path() const;
    QString versionString() const;

private:
    friend ModList;
    std::shared_ptr<Impl> impl_;

    // Wrap the pointer manually, to preserve CopyConstructable/CopyAssignable.
    inline const Impl *impl() const { return impl_.get(); };
    inline Impl *impl() { return impl_.get(); };
};

}  // namespace iimodmanager

#endif // MODLIST_H
