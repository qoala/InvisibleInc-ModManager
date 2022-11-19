#include "markupdatescommand.h"
#include "modmanguiapplication.h"
#include "modspecpreviewmodel.h"

#include <modcache.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

MarkUpdatesCommand::MarkUpdatesCommand(ModManGuiApplication &app, ModSpecPreviewModel *preview, QObject *parent)
    : QObject(parent), app(app), preview(preview)
{}

void MarkUpdatesCommand::execute()
{
    app.refreshMods();

    QList<SpecMod> specs;
    specs.reserve(app.modList().mods().size());

    for (const InstalledMod &im : app.modList().mods())
    {
        const CachedMod *cm = app.cache().mod(im.id());
        const CachedVersion *cv = cm ? cm->latestVersion() : nullptr;
        if (cv && !cv->installed())
            specs << im.asSpec().withoutVersion();
    }

    preview->insertModSpec(specs);
    emit finished();
}

} // namespace iimodmanager
