#ifndef INSTALLEDSTATUSCOMMAND_H
#define INSTALLEDSTATUSCOMMAND_H

#include <QTextCursor>

namespace iimodmanager {

class ModManGuiApplication;


class InstalledStatusCommand : public QObject
{
    Q_OBJECT

public:
    InstalledStatusCommand(ModManGuiApplication &app, QTextCursor cursor, QObject *parent = nullptr);

    void execute();

signals:
    void finished();

private:

    ModManGuiApplication &app;
    QTextCursor cursor;
};

} // namespace iimodmanager

#endif // INSTALLEDSTATUSCOMMAND_H

