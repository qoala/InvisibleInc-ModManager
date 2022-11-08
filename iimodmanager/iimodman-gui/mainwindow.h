#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <QMainWindow>
#include <QMenu>
#include <QTabWidget>

namespace iimodmanager {

class ModManGuiApplication;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ModManGuiApplication &app);

private slots:
    void enableActions();
    void writeText(QString value);

    void openSettings();
    void cacheStatus();
    void cacheUpdate();
    void cacheSave();
    void cacheAddMod();
    void installedStatus();
    void installedUpdate();
    void installedSyncFile();

private:
    void createTabs();
    void createLogDock();
    void createMenuActions();
    void disableActions();

    ModManGuiApplication &app;

    QTabWidget *tabWidget;
    // TODO: Add the important elements from tab pages, when defined.

    QDockWidget *logDock;
    QPlainTextEdit *logDisplay;

    QMenu *fileMenu;
    QAction *settingsAct;
    QAction *quitAct;
    QMenu *cacheMenu;
    QAction *cacheStatusAct;
    QAction *cacheUpdateAct;
    QAction *cacheSaveAct;
    QAction *cacheAddModAct;
    QMenu *installedMenu;
    QAction *installedStatusAct;
    QAction *installedUpdateAct;
    QAction *installedSyncFileAct;
};

} // namespace iimodmanager

#endif // MAINWINDOW_H

