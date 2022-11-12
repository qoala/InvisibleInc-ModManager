#ifndef INSTALLEDSTATUSCOMMAND_H
#define INSTALLEDSTATUSCOMMAND_H

# include<QObject>

namespace iimodmanager {

class ModManGuiApplication;


class InstalledStatusCommand : public QObject
{
    Q_OBJECT

public:
    InstalledStatusCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);

private:
    ModManGuiApplication &app;
};

} // namespace iimodmanager

#endif // INSTALLEDSTATUSCOMMAND_H

