#ifndef GUIDOWNLOADER_H
#define GUIDOWNLOADER_H

#include <QObject>
#include <moddownloader.h>

namespace iimodmanager {

class ModDownloadCall;
class ModManGuiApplication;
struct SteamModInfo;


class GuiModDownloader : public QObject
{
    Q_OBJECT

public:
    GuiModDownloader(ModManGuiApplication &app, const QList<SteamModInfo> &steamInfos, QObject *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);
    void beginProgress(int maximum);
    void updateProgress(int value);

private slots:
    void steamDownloadFinished();

private:
    const QList<SteamModInfo> steamInfos;

    ModDownloadCall *steamDownloadCall;
    qsizetype loopIndex;
};

} // namespace iimodmanager

#endif // GUIDOWNLOADER_H


