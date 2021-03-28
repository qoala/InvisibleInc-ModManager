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
