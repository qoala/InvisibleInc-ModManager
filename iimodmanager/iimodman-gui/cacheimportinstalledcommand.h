#ifndef CACHEIMPORTINSTALLEDCOMMAND_H
#define CACHEIMPORTINSTALLEDCOMMAND_H

#include "cacheimportmodel.h"

#include <QObject>
#include <QWidget>

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

private slots:
    void dialogFinished(int result);

private:
    ModManGuiApplication &app;
    CacheImportModel *model;
};

} // namespace iimodmanager

#endif // CACHEIMPORTINSTALLEDCOMMAND_H
