#include "confirmationprompt.h"
#include "modmancliapplication.h"
#include "modsremovecommand.h"

#include <QCommandLineParser>
#include <QTimer>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>

namespace iimodmanager {

ModsRemoveCommand::ModsRemoveCommand(ModManCliApplication &app)
    : Command(app) {}

void ModsRemoveCommand::addTerminalArgs(QCommandLineParser &parser) const
{
    parser.addPositionalArgument("remove", "Command: Uninstall the specified mods.");
    parser.addOptions({
                          {{"m", "mod-id"}, "Remove mods by ID (e.g. 'workshop-2151835746'), may be repeated.", "id"},
                      });
}

void ModsRemoveCommand::parse(QCommandLineParser &parser, const QStringList &args)
{
    Q_UNUSED(args);

    if (parser.isSet("mod-id"))
        modIds.append(parser.values("mod-id"));
}

void ModsRemoveCommand::execute()
{
    cache = new ModCache(app_.config(), this);
    modList = new ModList(app_.config(), cache, this);
    cache->refresh(ModCache::LATEST_ONLY);
    modList->refresh();

    mods.clear();
    mods.reserve(modIds.size());
    for (const QString &modId : modIds)
    {
        const InstalledMod *im = modList->mod(modId);
        if (im)
            mods.append(*im);
    }

    QTextStream cerr(stderr);
    if (mods.empty())
    {
        cerr << "No mods to remove." << Qt::endl;
        QTimer::singleShot(0, this, &Command::finished);
        return;
    }

    cerr << "The following mods will be removed:" << Qt::endl << "  ";
    for (const InstalledMod &im : mods)
        cerr << QString("\"%1\" [%2] ").arg(im.info().name(), im.id());
    cerr << Qt::endl;

    prompt = new ConfirmationPrompt(this);
    connect(prompt, &ConfirmationPrompt::yes, this, &ModsRemoveCommand::doRemoves);
    connect(prompt, &ConfirmationPrompt::no, this, [this] {
        QTextStream(stderr) << "Abort." << Qt::endl;
        emit finished();
    });
    prompt->start();
}

void ModsRemoveCommand::doRemoves()
{
    for (const InstalledMod &im : mods)
    {
        if (modList->removeMod(im.id()))
        {
            QTextStream(stderr) << im.info().toString() << " removed" << Qt::endl;
        }
        else
        {
            QTextStream(stderr) << "Failed to remove " << im.info().toString() << Qt::endl;
            app_.exit(EXIT_FAILURE);
            return;
        }
    }
    emit finished();
}

} // namespace iimodmanager
