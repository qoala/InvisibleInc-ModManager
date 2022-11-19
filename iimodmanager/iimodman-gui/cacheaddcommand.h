#ifndef CACHEADDCOMMAND_H
#define CACHEADDCOMMAND_H

#include <QObject>
#include <moddownloader.h>

namespace iimodmanager {

class ModDownloadCall;
class ModInfoCall;
class ModManGuiApplication;
struct SteamModInfo;


class CacheAddCommand : public QObject
{
    Q_OBJECT

public:
    CacheAddCommand(ModManGuiApplication &app, QWidget *parent = nullptr);

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
    ModManGuiApplication &app;

    ModInfoCall *steamInfoCall;
    QStringList workshopIds;
    QList<SteamModInfo> steamInfos;
    qsizetype loopIndex;

    QString parseIds(QString input, QStringList *modIds, bool *ok = nullptr, QStringList *failedIds = nullptr);
    void filterNewWorkshopIds(QStringList modIds);

    void startInfos();
    void nextSteamInfo();
    void startDownloads();
};

} // namespace iimodmanager

#endif // CACHEADDCOMMAND_H

