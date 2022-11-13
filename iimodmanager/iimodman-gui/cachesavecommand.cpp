#include "cachesavecommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <QFileDialog>
#include <QTextStream>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

CacheSaveCommand::CacheSaveCommand(ModManGuiApplication  &app, SaveOptions options, QObject *parent)
  : QObject(parent), app(app), options(options)
{}

void CacheSaveCommand::execute()
{
    app.refreshMods();

    QString content = generateContent();
    writeFile(content);

    emit finished();
    deleteLater();
}

void writeHeader(QTextStream &out)
{
    out << QStringLiteral("# II Mod Manager mod-spec") << Qt::endl;
    out << QStringLiteral("# Blank lines and lines starting with '#' are ignored") << Qt::endl;
    out << QStringLiteral("#") << Qt::endl;
    out << QStringLiteral("# Format: '{mod-id}:{install-alias}:{version-id}::{mod-name}") << Qt::endl;
    out << QStringLiteral("#   mod-id: The ID of the mod in the cache (do not modify this)") << Qt::endl;
    out << QStringLiteral("#   install-alias: The folder name to install as, if different from mod-id (usually left blank)") << Qt::endl;
    out << QStringLiteral("#   version-id: The version ID in the cache, overriding the latest version (usually left blank)") << Qt::endl;
    out << QStringLiteral("#   mod-name: Ignored. Provided for human readability.") << Qt::endl;
    out << QStringLiteral("#") << Qt::endl;
    out << QStringLiteral("# To get started, delete or comment out lines below, leaving just the mods you want to install.") << Qt::endl;
    out << QStringLiteral("#") << Qt::endl;
}

QString CacheSaveCommand::generateContent() const
{
    QString content;
    QTextStream out(&content);

    if (options.testFlag(CACHED_MODS))
    {
        QList<CachedMod> mods(app.cache().mods());
        content.reserve(mods.size() * 50 + 600);
        writeHeader(out);

        std::sort(mods.begin(), mods.end(), util::compareCachedModNames);
        for (const auto &mod : mods) {
            if (mod.downloaded()) {
                SpecMod spec = mod.latestVersion()->asSpec();
                if (options.testFlag(WITH_VERSIONS))
                    out << spec.asVersionedSpecString() << Qt::endl;
                else
                    out << spec.asSpecString() << Qt::endl;
            }
        }
    }
    else if (options.testFlag(INSTALLED_MODS))
    {
        QList<InstalledMod> mods(app.modList().mods());
        content.reserve(mods.size() * 50 + 600);
        writeHeader(out);

        std::sort(mods.begin(), mods.end(), util::compareInstalledModNames);
        for (const auto &mod : mods) {
            SpecMod spec = mod.asSpec();
            if (options.testFlag(WITH_VERSIONS))
                out << spec.asVersionedSpecString() << Qt::endl;
            else
                out << spec.asSpecString() << Qt::endl;
        }
    }

    return content;
}

void CacheSaveCommand::writeFile(QString content)
{
    QFileDialog::saveFileContent(content.toUtf8(), "mods-all.txt");
}

} // namespace iimodmanager

