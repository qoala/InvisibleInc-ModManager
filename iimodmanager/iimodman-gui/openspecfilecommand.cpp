#include "modmanguiapplication.h"
#include "openspecfilecommand.h"

#include <QFileDialog>
#include <modcache.h>
#include <modspec.h>

namespace iimodmanager {

OpenSpecFileCommand::OpenSpecFileCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app)
{
    inputSpec = new ModSpec(this);
}

void OpenSpecFileCommand::execute()
{
    auto fileContentReady = [this](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty())
        {
            // Selection cancelled
            deleteLater();
        }
        else
        {
            handleFile(fileName, fileContent);
        }
    };
    QFileDialog::getOpenFileContent("Mod Specs (*.txt)", fileContentReady);
}

void OpenSpecFileCommand::handleFile(const QString &filename, const QByteArray &fileContent)
{
    emit started();
    app.refreshMods();

    inputSpec->reserve(app.cache().mods().size());
    inputSpec->appendFromFile(fileContent, filename);

    emit modSpecReady(inputSpec->mods());
    emit finished();
    deleteLater();
}

} // namespace iimodmanager
