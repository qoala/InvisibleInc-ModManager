#ifndef CACHEUPDATECOMMAND_H
#define CACHEUPDATECOMMAND_H

#include <QObject>
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
    CacheUpdateCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void textOutput(QString value);
    void finished();

private slots:
    void steamInfoFinished();
    void steamDownloadFinished();

private:
    void startInfos();
    void nextSteamInfo();
    void startDownloads();
    void nextDownload();

    ModManGuiApplication &app;

    ModInfoCall *steamInfoCall;
    ModDownloadCall *steamDownloadCall;
    QStringList workshopIds;
    QList<SteamModInfo> steamInfos;
    qsizetype loopIndex;
};

} // namespace iimodmanager

#endif // CACHEUPDATECOMMAND_H
