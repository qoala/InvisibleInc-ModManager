#include "modmancliapplication.h"
#include "commandparser.h"

#include <QTextStream>

namespace iimodmanager {

ModManCliApplication::ModManCliApplication(int &argc, char **argv[])
    : app_(argc, *argv), config_()
{
    app_.setApplicationName(ModManConfig::organizationName);
    app_.setOrganizationName(ModManConfig::applicationName);
}

int ModManCliApplication::main(int argc, char *argv[])
{
    ModManCliApplication app(argc, &argv);

    // TODO: Examine arguments and do things based on that.
    CommandParser parser(app);
    parser.parse(app.app_.arguments());
    return 0;
}

}  // namespace iimodmanager
