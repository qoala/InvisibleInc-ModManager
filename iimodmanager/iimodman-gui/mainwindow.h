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

class ModManGuiApplication;
class ModsModel;
class ModsSortFilterProxyModel;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ModManGuiApplication &app);

private slots:
    void modsFilterStatusChanged();

    //! Triggered when an action begins making changes. Must be followed by actionFinished.
    //! For most actions, this is called by MainWindow before starting the action.
    void actionStarted();
    //! Triggered when an action finishes making changes.
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
    ModsModel *modsModel;
    ModsSortFilterProxyModel *modsSortFilterProxy;
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

