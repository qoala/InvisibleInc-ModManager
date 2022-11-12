#include "cachesavecommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <QFileDialog>
#include <QTextStream>
#include <modcache.h>
#include <modinfo.h>
#include <modspec.h>

namespace iimodmanager {

CacheSaveCommand::CacheSaveCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app)
{}

void CacheSaveCommand::execute()
{
    app.refreshMods();

    QString content = generateContent();
    writeFile(content);

    emit finished();
    deleteLater();
}

QString CacheSaveCommand::generateContent()
{
    QList<CachedMod> mods(app.cache().mods());
    std::sort(mods.begin(), mods.end(), util::compareCachedModNames);
    QString content;
    content.reserve(mods.size() * 50 + 600);
    QTextStream out(&content);
    out << "# II Mod Manager mod-spec" << Qt::endl;
    out << "# Blank lines and lines starting with '#' are ignored" << Qt::endl;
    out << "#" << Qt::endl;
    out << "# Format: '{mod-id}:{install-alias}:{version-id}::{mod-name}" << Qt::endl;
    out << "#   mod-id: The ID of the mod in the cache (do not modify this)" << Qt::endl;
    out << "#   install-alias: The folder name to install as, if different from mod-id (usually left blank)" << Qt::endl;
    out << "#   version-id: The version ID in the cache, overriding the latest version (usually left blank)" << Qt::endl;
    out << "#" << Qt::endl;
    out << "# To get started, delete or comment out lines below, leaving just the mods you want to install." << Qt::endl;
    out << "#" << Qt::endl;
    for (auto mod : mods) {
        if (mod.downloaded()) {
            out << mod.latestVersion()->asSpec().asSpecString() << Qt::endl;
        }
    }
    return content;
}

void CacheSaveCommand::writeFile(QString content)
{
    QFileDialog::saveFileContent(content.toUtf8(), "mods-all.txt");
}

} // namespace iimodmanager

