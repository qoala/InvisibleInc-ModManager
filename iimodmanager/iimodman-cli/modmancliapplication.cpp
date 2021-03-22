#include "modmancliapplication.h"
#include "commandparser.h"

#include <QFutureWatcher>
#include <QTextStream>

namespace iimodmanager {

ModManCliApplication::ModManCliApplication(int &argc, char **argv[])
    : QCoreApplication(argc, *argv), config_()
{
    setApplicationName(ModManConfig::organizationName);
    setOrganizationName(ModManConfig::applicationName);
}

int ModManCliApplication::main(int argc, char *argv[])
{
    ModManCliApplication app(argc, &argv);

    qSetMessagePattern("[%{type} %{time}] %{if-category}%{category}: %{endif}%{message}");

    CommandParser parser(app);
    QFuture<void> future = parser.parse(app.arguments());

    QFutureWatcher<void> watcher;
    connect(&watcher, &QFutureWatcher<void>::finished, &app, &QCoreApplication::quit);
    watcher.setFuture(future);

    return app.exec();
}

}  // namespace iimodmanager
