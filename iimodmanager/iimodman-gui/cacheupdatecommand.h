#ifndef CACHEUPDATECOMMAND_H
#define CACHEUPDATECOMMAND_H

#include <QTextCursor>
#include <moddownloader.h>

namespace iimodmanager {

class ModDownloadCall;
class ModInfoCall;
class ModManGuiApplication;
class SteamModInfo;


class CacheUpdateCommand : public QObject
{
    Q_OBJECT

public:
    CacheUpdateCommand(ModManGuiApplication &app, QTextCursor cursor, QObject *parent = nullptr);

    void execute();

signals:
    void finished();

private slots:
    void steamInfoFinished();

private:
    void startInfos();
    void nextSteamInfo();

    ModManGuiApplication &app;
    QTextCursor cursor;

    ModInfoCall *steamInfoCall;
    ModDownloadCall *steamDownloadCall;
    QStringList workshopIds;
    QList<SteamModInfo> steamInfos;
    qsizetype loopIndex;
};

} // namespace iimodmanager

#endif // CACHEUPDATECOMMAND_H

