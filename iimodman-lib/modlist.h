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
struct SteamModInfo;


class InstalledMod;

//! Mods installed to the Invisible Inc application.
class IIMODMANLIBSHARED_EXPORT ModList : public QObject
{
    Q_OBJECT

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

    ModList(const ModManConfig &config, ModCache *cache, QObject *parent = nullptr);

    const QList<InstalledMod> &mods() const;
    bool contains(const QString &id) const;
    const InstalledMod *mod(const QString &id) const;


    void refresh(RefreshLevel level = FULL);
    //! Installs the specified mod from the cache.
    //! Does not re-sort the mods list.
    const InstalledMod *installMod(const SpecMod &specMod, QString *errorInfo = nullptr);
    //! Uninstalls the specified mod.
    //! Requires a refresh before the removal is reflected in the mods list and cache.
    bool removeMod(const QString &modId, QString *errorInfo = nullptr);

    ~ModList();

signals:
    //! Emitted before mods are arbitrarily changed.
    void aboutToRefresh();
    //! Emitted after mods are arbitrarily changed.
    void refreshed();

private:
    std::experimental::propagate_const<std::unique_ptr<Impl>> impl;
};

class IIMODMANLIBSHARED_EXPORT InstalledMod
{
public:
    //! Private implementation. Only accessible to classes in this file.
    class Impl;

    InstalledMod(ModList::Impl &parent, const QString &id);

    // Mod ID of the installed mod. Always matches the cache ID, even if installed with an alias.
    const QString &id() const;
    // Installed alias, or null string if installed by its ID.
    const QString &alias() const;
    const QString &installedId() const { return alias().isEmpty() ? id() : alias(); }
    const ModInfo &info() const;
    const QString &hash() const;

    bool hasCacheVersion() const;
    //! The cached version matching the currently installed mod, or nullptr if there is no match.
    const CachedVersion *cacheVersion() const;
    //! Attempts to match a cache version by a different mod ID.
    const CachedVersion *alternateCacheVersion(const QString &cacheId) const;

    const SpecMod asSpec() const;
    QString path() const;
    QString versionString() const;

    //! Write a metadata file claiming the currently installed mod is for the given cache mod, which may have a different ID.
    //! The cache mod ID is used as-is if it exists in the cache, but the version ID will be re-checked on hash.
    //! Requires a refresh before the removal is reflected in the mods list and cache.
    bool writeMetadataClaim(const QString &cacheId, const QString &cacheVersionId = QString()) const;

private:
    friend ModList;
    std::shared_ptr<Impl> impl_;

    // Wrap the pointer manually, to preserve CopyConstructable/CopyAssignable.
    inline const Impl *impl() const { return impl_.get(); };
    inline Impl *impl() { return impl_.get(); };
};

}  // namespace iimodmanager

#endif // MODLIST_H
