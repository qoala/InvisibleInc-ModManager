#include "guiconfig.h"

#include <QSettings>
#include <QString>
#include <modmanconfig.h>

namespace iimodmanager {

static const QString openMaximizedKey = QStringLiteral("gui/openMaximized");

const bool GuiConfig::openMaximized() const
{
    return settings_.value(openMaximizedKey, false).toBool();
}

void GuiConfig::setOpenMaximized(bool value)
{
    settings_.setValue(openMaximizedKey, value);
}

}
