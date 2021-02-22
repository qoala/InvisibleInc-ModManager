#include "mod.h"

#include <QDir>
#include <QRegularExpression>
#include <QSettings>
#include <QTextStream>

namespace iimodmanager {

Mod::Mod(const QString &id, ModLocation location, const QString &name)
    : id_(id), location_(location), name_(name)
{}

const Mod Mod::readModInfo(const ModManConfig &config, const QString &id, ModLocation location)
{
    const QDir modsDir(location == CACHED ? config.cachePath() : config.installPath());
    assert(modsDir.exists());
    const QDir modDir(modsDir.filePath(id));
    assert(modsDir.exists());
    const QFileInfo modInfo(modDir, "modinfo.txt");
    assert(modInfo.isFile());

    QString name;

    QFile file(modInfo.absoluteFilePath());
    if (file.open(QIODevice::ReadOnly))
    {
        const QRegularExpression linePattern("^\\s*(\\w+)\\s*=\\s*(.*)$");
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QRegularExpressionMatch match = linePattern.match(line);
            if (match.hasMatch() && match.captured(1) == "name")
            {
                name = match.captured(2);
            }
        }
    }

    return Mod(id, location, name);
}


}  // namespace iimodmanager
