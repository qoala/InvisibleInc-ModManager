#include "command.h"
#include "modmancliapplication.h"

#include <QCommandLineParser>

namespace iimodmanager {

Command::~Command()
{}

Command::Command(ModManCliApplication &app)
    : app_(app)
{}

}  // namespace iimodmanager
