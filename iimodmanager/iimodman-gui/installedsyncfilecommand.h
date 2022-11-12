#ifndef INSTALLEDSYNCFILECOMMAND_H
#define INSTALLEDSYNCFILECOMMAND_H

# include<QObject>
# include<modlist.h>
# include<modspec.h>

namespace iimodmanager {

class ModManGuiApplication;


class InstalledSyncFileCommand : public QObject
{
    Q_OBJECT

public:
    InstalledSyncFileCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);

private:
    void handleFile(const QString &filename, const QByteArray &fileContent);
    std::optional<SpecMod> makeInstallTarget(const SpecMod &specMod);
    bool checkInstalledMod(const InstalledMod &installedMod);
    bool doSync();
    bool removeMod(const InstalledMod &installedMod);
    bool installMod(const SpecMod &specMod);
    bool updateMod(const SpecMod &specMod);

    ModManGuiApplication &app;

    // ModSpec as read from input
    ModSpec inputSpec;
    // ModSpec with exact versions specified
    ModSpec targetSpec;
    // Mods that need to be newly installed
    QList<SpecMod> addedMods;
    // Mods that need to be updated
    QList<SpecMod> updatedMods;
    // Mods that need to be removed
    QList<InstalledMod> removedMods;
};

} // namespace iimodmanager

#endif // INSTALLEDSYNCFILECOMMAND_H


