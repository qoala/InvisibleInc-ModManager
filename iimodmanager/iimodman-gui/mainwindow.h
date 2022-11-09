#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QDockWidget>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QPlainTextEdit>
#include <QSortFilterProxyModel>
#include <QTabWidget>
#include <QTreeView>

namespace iimodmanager {

class ModCacheModel;
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
    ModCacheModel *cacheModel;
    QSortFilterProxyModel *cacheProxyModel;
    QTreeView *cacheView;
    QLineEdit *cacheSearchInput;

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

