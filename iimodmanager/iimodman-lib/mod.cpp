#include "mod.h"

#include <QDir>
#include <QRegularExpression>
#include <QSettings>
#include <QTextStream>

namespace iimodmanager {

Mod::Mod()
{}

Mod::Mod(const QString &id)
    : id_(id)
{}

const Mod Mod::readModInfo(const ModManConfig &config, const QString &id, ModLocation location)
{
    const QDir modsDir(location == CACHED ? config.cachePath() : config.installPath());
    assert(modsDir.exists());
    const QDir modDir(modsDir.filePath(id));
    assert(modsDir.exists());
    const QFileInfo modInfo(modDir, "modinfo.txt");
    assert(modInfo.isFile());

    QFile file(modInfo.absoluteFilePath());
    return readModInfo(file, id);
}

const Mod Mod::readModInfo(QIODevice &file, const QString &id)
{

    Mod mod(id);

    if (file.isOpen() || file.open(QIODevice::ReadOnly))
    {
        const QRegularExpression linePattern("^\\s*(\\w+)\\s*=\\s*(.*)$");
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QRegularExpressionMatch match = linePattern.match(line);
            if (match.hasMatch())
            {
                QString key = match.captured(1);
                if (key == "name")
                {
                    mod.name_ = match.captured(2);
                }
                else if (key == "version")
                {
                    mod.version_ = match.captured(2);
                }
            }
        }
    }

    return mod;
}

}  // namespace iimodmanager
