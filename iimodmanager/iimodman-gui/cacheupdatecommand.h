#ifndef CACHEUPDATECOMMAND_H
#define CACHEUPDATECOMMAND_H

#include <QObject>
#include <moddownloader.h>

namespace iimodmanager {

class ModInfoCall;
class ModManGuiApplication;
struct SteamModInfo;


class CacheUpdateCommand : public QObject
{
    Q_OBJECT

public:
    CacheUpdateCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);
    void beginProgress(int maximum);
    void updateProgress(int value);

private slots:
    void steamInfoFinished();
    void modDownloadFinished();

private:
    void startInfos();
    void nextSteamInfo();
    void startDownloads();

    ModManGuiApplication &app;

    ModInfoCall *steamInfoCall;
    QStringList workshopIds;
    QList<SteamModInfo> steamInfos;
    qsizetype loopIndex;
};

} // namespace iimodmanager

#endif // CACHEUPDATECOMMAND_H

