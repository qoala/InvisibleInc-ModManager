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
#include <QPushButton>
#include <QTabWidget>
#include <QTreeView>

namespace iimodmanager {

class ModManGuiApplication;
class ModSpecPreviewModel;
class ModsSortFilterProxyModel;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ModManGuiApplication &app);

private slots:

    // Action callbacks.
    //! Triggered when an action begins making changes. Must be followed by actionFinished.
    //! For most actions, this is called by MainWindow before starting the action.
    void actionStarted();
    //! Triggered when an action finishes making changes.
    void actionFinished();
    void writeText(QString value);
    //! Start a progress bar. May be called multiple times before calling endProgress.
    void beginProgress(int maximum);
    //! Completes and hides the progress bar. Automatically triggered by actionFinished.
    void endProgress();

    // File Menu
    void openSpec();
    void saveInstalledSpec();
    void saveInstalledVersionSpec();
    void saveCacheSpec();
    void openSettings();
    // Cache Menu
    void cacheStatus();
    void cacheUpdate();
    void cacheAddMod();
    // Installed Menu
    void installedStatus();
    void installedUpdate();

    // Main Page
    void modsFilterStatusChanged();
    void updatePreviewActionsEnabled();
    void applyPreview();
    void revertPreview();

private:
    ModManGuiApplication &app;
    //! True if an action is currently modifying the cached/installed mods.
    bool isLocked_;

    QTabWidget *tabWidget;
    ModSpecPreviewModel *modsPreviewModel;
    ModsSortFilterProxyModel *modsSortFilterProxy;
    QTreeView *modsView;
    QLineEdit *modsSearchInput;
    QCheckBox *modsInstalledCheckBox;
    QCheckBox *modsHasCachedUpdateCheckBox;

    QDockWidget *logDock;
    QPlainTextEdit *logDisplay;
    QProgressBar *logProgress;
    QPushButton *applyPreviewBtn;
    QPushButton *revertPreviewBtn;

    QMenu *fileMenu;
    QAction *openSpecAct;
    QAction *saveInstalledSpecAct;
    QAction *saveInstalledVersionSpecAct;
    QAction *saveCacheSpecAct;
    QAction *settingsAct;
    QAction *quitAct;
    QMenu *cacheMenu;
    QAction *cacheStatusAct;
    QAction *cacheUpdateAct;
    QAction *cacheAddModAct;
    QMenu *installedMenu;
    QAction *installedStatusAct;
    QAction *installedUpdateAct;

    inline bool isLocked() const { return isLocked_; };
    inline void setLock(bool locked) { isLocked_ = locked; setActionsEnabled(!locked); };

    void createTabs();
    void createLogDock();
    void createMenuActions();
    void setActionsEnabled(bool enabled);
};

} // namespace iimodmanager

#endif // MAINWINDOW_H

