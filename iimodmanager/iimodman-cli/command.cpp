#include "command.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>
#include <QFuture>

namespace iimodmanager {

Command::~Command()
{}

Command::Command(ModManCliApplication &app)
    : app_(app)
{}

}  // namespace iimodmanager
