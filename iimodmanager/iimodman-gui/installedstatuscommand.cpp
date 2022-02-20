#include "installedstatuscommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <modinfo.h>

namespace iimodmanager {

InstalledStatusCommand::InstalledStatusCommand(ModManGuiApplication  &app, QTextCursor cursor, QObject *parent)
  : QObject(parent), app(app), cursor(cursor)
{}

void InstalledStatusCommand::execute()
{
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n\n---\nCurrently Installed Mods\n\n");

    QList<InstalledMod> mods(app.modList().mods());
    std::sort(mods.begin(), mods.end(), util::compareModNames);
    for (auto mod : mods) {
        const ModInfo &info = mod.info();

        cursor.insertText(info.name());
        cursor.insertText("\t");
        cursor.insertText(info.id());
        cursor.insertText("\t");
        cursor.insertText(mod.versionString());
        if (!mod.hasCacheVersion())
        {
            cursor.insertText("\t");
            cursor.insertText("(not in cache)");
        }
        cursor.insertBlock();
    }

    emit finished();
    deleteLater();
}

} // namespace iimodmanager
