#include "installedsyncfilecommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <QFileDialog>
#include <modinfo.h>

namespace iimodmanager {

InstalledSyncFileCommand::InstalledSyncFileCommand(ModManGuiApplication  &app, QObject *parent)
  : QObject(parent), app(app)
{}

void InstalledSyncFileCommand::execute()
{
    app.cache().refresh(ModCache::LATEST_ONLY);
    app.modList().refresh();

    auto fileContentReady = [this](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty())
        {
            // Selection cancelled
            emit finished();
            deleteLater();
        }
        else
        {
            handleFile(fileName, fileContent);
        }
    };
    QFileDialog::getOpenFileContent("Modspecs (*.txt)", fileContentReady);
}

void InstalledSyncFileCommand::handleFile(const QString &filename, const QByteArray &fileContent)
{
    inputSpec.reserve(app.cache().mods().size());
    inputSpec.appendFromFile(fileContent, filename);

    emit textOutput("\n--");
    for (const auto mod : inputSpec.mods())
        emit textOutput(mod.name());

    emit finished();
    deleteLater();
}

} // namespace iimodmanager

