#ifndef APPLYPREVIEWCOMMAND_H
#define APPLYPREVIEWCOMMAND_H

#include "modspecpreviewmodel.h"

#include <QObject>

namespace iimodmanager {

class ModManGuiApplication;


class ApplyPreviewCommand : public QObject
{
    Q_OBJECT

public:
    ApplyPreviewCommand(ModManGuiApplication &app, ModSpecPreviewModel *preview, QObject *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);

private:
    ModManGuiApplication &app;
    ModSpecPreviewModel *preview;

    void finish();
    bool doSync();
    bool removeMod(const InstalledMod &installedMod);
    bool installMod(const SpecMod &specMod);
    bool updateMod(const SpecMod &specMod);
};

} // namespace iimodmanager

#endif // APPLYPREVIEWCOMMAND_H
