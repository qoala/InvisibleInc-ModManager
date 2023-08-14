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

signals:
    void lockChanged(bool locked);

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
    void openSettings(bool isStartup = false);
    // Cache Menu
    void cacheCheckDlUpdate();
    void cacheDownloadUpdate();
    void markUpdates();
    void cacheAddMod();
    void cacheImportInstalled(bool isStartup = false);

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
    QAction *cacheCheckDlUpdateAct;
    QAction *cacheDownloadUpdateAct;
    QAction *markUpdatesAct;
    QAction *cacheAddModAct;
    QAction *cacheImportInstalledAct;

    inline bool isLocked() const { return isLocked_; };
    inline void setLock(bool locked)
    { isLocked_ = locked; setActionsEnabled(!locked); emit lockChanged(locked); };

    void createTabs();
    void createLogDock();
    void createMenuActions();
    void setActionsEnabled(bool enabled);
    void onStartup(int stage);

    void onNewAppVersion(QString version, QString url, const QString errorInfo);
};

} // namespace iimodmanager

#endif // MAINWINDOW_H

