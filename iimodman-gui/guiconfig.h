#ifndef GUICONFIG_H
#define GUICONFIG_H

#include <modmanconfig.h>


namespace iimodmanager {

class GuiConfig : public ModManConfig
{
public:
    const bool openMaximized() const;
    void setOpenMaximized(bool);
};

}  // namespace iimodmanager

#endif // GUICONFIG_H

