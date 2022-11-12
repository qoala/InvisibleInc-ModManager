#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QCheckBox>
#include <QDockWidget>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QTabWidget>
#include <QTreeView>

namespace iimodmanager {

class ModCacheModel;
class ModManGuiApplication;
class ModSortFilterProxyModel;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ModManGuiApplication &app);

private slots:
    void modsFilterStatusChanged();

    void actionFinished();
    void writeText(QString value);
    //! Start a progress bar. May be called multiple times without finishing.
    void beginProgress(int maximum);
    void endProgress();

    void openSettings();
    void cacheStatus();
    void cacheUpdate();
    void cacheSave();
    void cacheAddMod();
    void installedStatus();
    void installedUpdate();
    void installedSyncFile();

private:
    ModManGuiApplication &app;

    QTabWidget *tabWidget;
    ModCacheModel *modsModel;
    ModSortFilterProxyModel *modsSortFilterProxy;
    QTreeView *modsView;
    QLineEdit *modsSearchInput;
    QCheckBox *modsInstalledCheckBox;
    QCheckBox *modsHasCachedUpdateCheckBox;

    QDockWidget *logDock;
    QPlainTextEdit *logDisplay;
    QProgressBar *logProgress;

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

    void createTabs();
    void createLogDock();
    void createMenuActions();
    void setActionsEnabled(bool enabled);
};

} // namespace iimodmanager

#endif // MAINWINDOW_H

