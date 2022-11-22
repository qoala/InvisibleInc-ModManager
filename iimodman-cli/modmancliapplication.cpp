#include "modmancliapplication.h"
#include "commandparser.h"
#include "command.h"

#include <QTextStream>

namespace iimodmanager {

ModManCliApplication::ModManCliApplication(int &argc, char **argv[])
    : QCoreApplication(argc, *argv), config_()
{
    setApplicationName(ModManConfig::applicationName);
    setOrganizationName(ModManConfig::organizationName);
}

int ModManCliApplication::main(int argc, char *argv[])
{
    ModManCliApplication app(argc, &argv);

    qSetMessagePattern("[%{type} %{time}] %{if-category}%{category}: %{endif}%{message}");

    CommandParser parser(app);
    Command *command = parser.parse(app.arguments());

    connect(command, &Command::finished, &app, &QCoreApplication::quit);

    command->execute();
    return app.exec();
}

}  // namespace iimodmanager
