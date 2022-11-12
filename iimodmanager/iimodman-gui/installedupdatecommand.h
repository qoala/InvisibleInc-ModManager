#ifndef INSTALLEDUPDATECOMMAND_H
#define INSTALLEDUPDATECOMMAND_H

# include<QObject>

namespace iimodmanager {

class CachedMod;
class ModManGuiApplication;
class SpecMod;


class InstalledUpdateCommand : public QObject
{
    Q_OBJECT

public:
    InstalledUpdateCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);

private:
    bool installMod(SpecMod sm, const CachedMod *from);

    ModManGuiApplication &app;
};

} // namespace iimodmanager

#endif // INSTALLEDUPDATECOMMAND_H


