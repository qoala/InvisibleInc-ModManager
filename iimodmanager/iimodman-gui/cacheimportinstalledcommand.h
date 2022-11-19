#ifndef CACHEIMPORTINSTALLEDCOMMAND_H
#define CACHEIMPORTINSTALLEDCOMMAND_H

#include "cacheimportmodel.h"

#include <QObject>
#include <QWidget>
#include <utility>

namespace iimodmanager {

class ModManGuiApplication;


class CacheImportInstalledCommand : public QObject
{
    Q_OBJECT

public:
    CacheImportInstalledCommand(ModManGuiApplication &app, QWidget *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);
    void beginProgress(int maximum);
    void updateProgress(int value);

private slots:
    void dialogFinished(int result);
    void modDownloadFinished();

private:
    ModManGuiApplication &app;
    CacheImportModel *model;

    QList<std::pair<QString, QString>> toCopyMods;

    void startCopyImports();
    bool copyMod(const QString &installedId, const QString &targetId);
};

} // namespace iimodmanager

#endif // CACHEIMPORTINSTALLEDCOMMAND_H
