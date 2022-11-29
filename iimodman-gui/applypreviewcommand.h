#ifndef APPLYPREVIEWCOMMAND_H
#define APPLYPREVIEWCOMMAND_H

#include "modspecpreviewmodel.h"

#include <QObject>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

class ModManGuiApplication;


class ApplyPreviewCommand : public QObject
{
    Q_OBJECT

public:
    ApplyPreviewCommand(ModManGuiApplication &app, ModSpecPreviewModel *preview, QWidget *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);

private:
    ModManGuiApplication &app;
    ModSpecPreviewModel *preview;

    QList<SpecMod> toAddMods;
    QList<SpecMod> toUpdateMods;
    QList<InstalledMod> toRemoveMods;

    void finish();
    void dialogFinished(int result);
    void applyChanges();
    bool doApply();
    bool removeMod(const InstalledMod &installedMod);
    bool installMod(const SpecMod &specMod);
    bool updateMod(const SpecMod &specMod);
};

} // namespace iimodmanager

#endif // APPLYPREVIEWCOMMAND_H
