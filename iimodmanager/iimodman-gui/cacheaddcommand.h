#ifndef CACHEADDCOMMAND_H
#define CACHEADDCOMMAND_H

#include <QObject>
#include <moddownloader.h>

namespace iimodmanager {

class ModDownloadCall;
class ModInfoCall;
class ModManGuiApplication;
class SteamModInfo;


class CacheAddCommand : public QObject
{
    Q_OBJECT

public:
    CacheAddCommand(ModManGuiApplication &app, QWidget *parent = nullptr);

    void execute();

signals:
    void textOutput(QString value);
    void finished();

private slots:
    void steamInfoFinished();
    void steamDownloadFinished();

private:
    QWidget *parentWidget;
    ModManGuiApplication &app;

    ModInfoCall *steamInfoCall;
    ModDownloadCall *steamDownloadCall;
    QStringList workshopIds;
    QList<SteamModInfo> steamInfos;
    qsizetype loopIndex;

    QString parseIds(QString input, QStringList *modIds, bool *ok = nullptr, QStringList *failedIds = nullptr);
    void filterNewWorkshopIds(QStringList modIds);

    void startInfos();
    void nextSteamInfo();
    void startDownloads();
    void nextDownload();
};

} // namespace iimodmanager

#endif // CACHEADDCOMMAND_H
