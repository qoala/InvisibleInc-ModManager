#ifndef INSTALLEDUPDATECOMMAND_H
#define INSTALLEDUPDATECOMMAND_H

# include<QObject>

namespace iimodmanager {

class ModManGuiApplication;
class SpecMod;


class InstalledUpdateCommand : public QObject
{
    Q_OBJECT

public:
    InstalledUpdateCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void textOutput(QString value);
    void finished();

private:
    void installMod(SpecMod sm);

    ModManGuiApplication &app;
};

} // namespace iimodmanager

#endif // INSTALLEDUPDATECOMMAND_H


