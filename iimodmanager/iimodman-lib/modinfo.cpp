#include "modinfo.h"

#include <QDir>
#include <QRegularExpression>
#include <QSettings>
#include <QTextStream>

namespace iimodmanager {

ModInfo::ModInfo()
{}

ModInfo::ModInfo(const QString &id, const QString &name)
    : id_(id), name_(name)
{}

const ModInfo ModInfo::readModInfo(const ModManConfig &config, const QString &id, ModLocation location)
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

const ModInfo ModInfo::readModInfo(QIODevice &file, const QString &id)
{

    ModInfo mod;
    mod.id_ = id;

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
