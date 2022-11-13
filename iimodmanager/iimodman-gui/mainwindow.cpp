#include "cacheaddcommand.h"
#include "cachesavecommand.h"
#include "cachestatuscommand.h"
#include "cacheupdatecommand.h"
#include "installedstatuscommand.h"
#include "installedupdatecommand.h"
#include "installedsyncfilecommand.h"
#include "mainwindow.h"
#include "modelutil.h"
#include "modmanguiapplication.h"
#include "modsmodel.h"
#include "modssortfilterproxymodel.h"
#include "settingsdialog.h"

#include <QApplication>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QVBoxLayout>
#include <modcache.h>

namespace iimodmanager {

MainWindow::MainWindow(ModManGuiApplication &app)
    : app(app)
{
    createTabs();
    createLogDock();
    createMenuActions();

    setWindowTitle(tr("II Mod Manager"));
    resize(980, 460);
}

void MainWindow::createTabs()
{
    tabWidget = new QTabWidget;
    setCentralWidget(tabWidget);

    // Main mods tab.
    modsModel = new ModsModel(app.cache(), app.modList(), this);
    modsSortFilterProxy = new ModsSortFilterProxyModel(this);
    modsSortFilterProxy->setSourceModel(modsModel);
    modsSortFilterProxy->setFilterColumns({ModsModel::NAME, ModsModel::ID});
    modsSortFilterProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    modsSortFilterProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    modsView = new QTreeView;
    modsView->setModel(modsSortFilterProxy);
    modsView->header()->setStretchLastSection(false);
    modsView->setColumnWidth(ModsModel::NAME, 300);
    modsView->setColumnWidth(ModsModel::ID, 160);
    modsView->setColumnWidth(ModsModel::INSTALLED_VERSION, 110);
    modsView->setColumnWidth(ModsModel::LATEST_VERSION, 110);
    modsView->setColumnWidth(ModsModel::INSTALLED_VERSION_TIME, 160);
    modsView->setColumnWidth(ModsModel::CACHE_UPDATE_TIME, 160);
    modsView->sortByColumn(ModsModel::NAME, Qt::AscendingOrder);
    modsView->setSortingEnabled(true);

    modsSearchInput = new QLineEdit;
    connect(modsSearchInput, &QLineEdit::textChanged, modsSortFilterProxy, &QSortFilterProxyModel::setFilterFixedString);

    modsInstalledCheckBox = new QCheckBox(tr("Installed"));
    modsInstalledCheckBox->setTristate(true);
    modsInstalledCheckBox->setCheckState(Qt::PartiallyChecked);
    connect(modsInstalledCheckBox, &QCheckBox::stateChanged, this, &MainWindow::modsFilterStatusChanged);
    modsHasCachedUpdateCheckBox = new QCheckBox(tr("Update Ready"));
    modsHasCachedUpdateCheckBox->setToolTip(tr("A new version is downloaded and ready to install."));
    connect(modsHasCachedUpdateCheckBox, &QCheckBox::stateChanged, this, &MainWindow::modsFilterStatusChanged);

    QHBoxLayout *modsFilterBtnLayout = new QHBoxLayout;
    modsFilterBtnLayout->addWidget(modsInstalledCheckBox);
    modsFilterBtnLayout->addWidget(modsHasCachedUpdateCheckBox);
    QVBoxLayout *modsLayout = new QVBoxLayout;
    modsLayout->addWidget(modsSearchInput);
    modsLayout->addLayout(modsFilterBtnLayout);
    modsLayout->addWidget(modsView);
    QWidget *modsPage = new QWidget;
    modsPage->setLayout(modsLayout);
    tabWidget->addTab(modsPage, tr("Mods"));

    // TODO: expanded cache management tab.
}

void MainWindow::createLogDock()
{
    QString initialMessage = QString("Mod Manager loaded.  \tInstalled: %1 mods  \tCache: %2 mods").arg(app.modList().mods().size()).arg(app.cache().mods().size());

    logDisplay = new QPlainTextEdit(initialMessage);
    logDisplay->setReadOnly(true);
    logDisplay->setMaximumBlockCount(10000);
    logDisplay->setTabStopDistance(240);
    logDisplay->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    logDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    logProgress = new QProgressBar();
    logProgress->setVisible(false);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(logDisplay);
    layout->addWidget(logProgress);
    QWidget *dockContents = new QWidget;
    dockContents->setLayout(layout);
    dockContents->setMinimumWidth(50);
    dockContents->setMinimumHeight(50);

    QDockWidget *logDock = new QDockWidget(tr("Logs"), this);
    logDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    logDock->setFeatures(QDockWidget::DockWidgetMovable); // Not floatable, Not closable.
    logDock->setWidget(dockContents);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

void MainWindow::createMenuActions()
{
    loadSpecAct = new QAction(tr("&Open Spec File..."), this);
    loadSpecAct->setStatusTip(tr("Installs/Updates/Uninstalls mods to match a modspec file."));
    connect(loadSpecAct, &QAction::triggered, this, &MainWindow::loadSpec);
    saveInstalledSpecAct = new QAction(tr("&Save Installed Mods As..."), this);
    saveInstalledSpecAct->setShortcuts(QKeySequence::Save);
    saveInstalledSpecAct->setStatusTip(tr("Save a modspec file listing currently installed mods"));
    connect(saveInstalledSpecAct, &QAction::triggered, this, &MainWindow::saveInstalledSpec);
    saveInstalledVersionSpecAct = new QAction(tr("Save Installed Mod &Versions As..."), this);
    saveInstalledVersionSpecAct->setStatusTip(tr("Save a modspec file listing currently installed mods with the installed versions. Allows restoring exactly the same mod versions later if they remain in the download cache."));
    connect(saveInstalledVersionSpecAct, &QAction::triggered, this, &MainWindow::saveInstalledVersionSpec);
    saveCacheSpecAct = new QAction(tr("Save &All Mods As..."), this);
    saveCacheSpecAct->setStatusTip(tr("Save a modspec file listing all cached mods"));
    connect(saveCacheSpecAct, &QAction::triggered, this, &MainWindow::saveCacheSpec);

    settingsAct = new QAction(tr("&Preferences"), this);
    settingsAct->setShortcuts(QKeySequence::Preferences);
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);
    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    connect(quitAct, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    cacheStatusAct = new QAction(tr("&Log Status"), this);
    cacheStatusAct->setStatusTip(tr("List downloaded mods to the log."));
    connect(cacheStatusAct, &QAction::triggered, this, &MainWindow::cacheStatus);
    cacheUpdateAct = new QAction(tr("Download &Updates"), this);
    cacheUpdateAct->setStatusTip(tr("Check for and download updates for all mods in the cache"));
    connect(cacheUpdateAct, &QAction::triggered, this, &MainWindow::cacheUpdate);
    cacheAddModAct = new QAction(tr("&Add Workshop Mod"), this);
    cacheAddModAct->setStatusTip(tr("Download and add a mod to the cache"));
    connect(cacheAddModAct, &QAction::triggered, this, &MainWindow::cacheAddMod);

    installedStatusAct = new QAction(tr("&Log Status"), this);
    installedStatusAct->setStatusTip(tr("List currently installed mods to the log."));
    connect(installedStatusAct, &QAction::triggered, this, &MainWindow::installedStatus);
    installedUpdateAct = new QAction(tr("Install &Updates"), this);
    installedUpdateAct->setStatusTip(tr("Updates currently installed mods to their latest downloaded version."));
    connect(installedUpdateAct, &QAction::triggered, this, &MainWindow::installedUpdate);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(loadSpecAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveInstalledSpecAct);
    fileMenu->addAction(saveInstalledVersionSpecAct);
    fileMenu->addAction(saveCacheSpecAct);
    fileMenu->addSeparator();
    fileMenu->addAction(settingsAct);
    fileMenu->addAction(quitAct);
    QMenu *cacheMenu = menuBar()->addMenu(tr("&Downloaded"));
    cacheMenu->addAction(cacheStatusAct);
    cacheMenu->addAction(cacheUpdateAct);
    cacheMenu->addSeparator();
    cacheMenu->addAction(cacheAddModAct);
    QMenu *installedMenu = menuBar()->addMenu(tr("&Installed"));
    installedMenu->addAction(installedStatusAct);
    installedMenu->addAction(installedUpdateAct);
}

void MainWindow::setActionsEnabled(bool enabled)
{
    settingsAct->setEnabled(enabled);
    cacheStatusAct->setEnabled(enabled);
    cacheUpdateAct->setEnabled(enabled);
    saveCacheSpecAct->setEnabled(enabled);
    cacheAddModAct->setEnabled(enabled);
    installedStatusAct->setEnabled(enabled);
    installedUpdateAct->setEnabled(enabled);
    loadSpecAct->setEnabled(enabled);
}

void MainWindow::modsFilterStatusChanged()
{
    modelutil::Status requiredStatuses, maskedStatuses;

    switch (modsInstalledCheckBox->checkState())
    {
    case Qt::Unchecked:
        maskedStatuses |= modelutil::INSTALLED_STATUS;
        break;
    case Qt::Checked:
        requiredStatuses |= modelutil::INSTALLED_STATUS;
        break;
    case Qt::PartiallyChecked:
        break;
    }

    if (modsHasCachedUpdateCheckBox->isChecked())
        requiredStatuses |= modelutil::CAN_INSTALL_UPDATE_STATUS;

    modsSortFilterProxy->setFilterStatus(requiredStatuses, maskedStatuses);
}

void MainWindow::actionStarted()
{
    setActionsEnabled(false);
}

void MainWindow::actionFinished()
{
    setActionsEnabled(true);
    endProgress();
}

void MainWindow::writeText(QString value)
{
    logDisplay->appendPlainText(value);
}

void MainWindow::beginProgress(int maximum)
{
    logProgress->setMaximum(maximum);
    logProgress->reset();
    logProgress->setVisible(true);
}

void MainWindow::endProgress()
{
    logProgress->setVisible(false);
    logProgress->reset();
}

// File actions.

void MainWindow::loadSpec()
{
    InstalledSyncFileCommand *command = new InstalledSyncFileCommand(app, this);
    connect(command, &InstalledSyncFileCommand::started, this, &MainWindow::actionStarted);
    connect(command, &InstalledSyncFileCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledSyncFileCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::saveInstalledSpec()
{
    actionStarted();
    CacheSaveCommand *command = new CacheSaveCommand(app, CacheSaveCommand::INSTALLED_MODS, this);
    connect(command, &CacheSaveCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheSaveCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::saveInstalledVersionSpec()
{
    actionStarted();
    CacheSaveCommand *command = new CacheSaveCommand(app, CacheSaveCommand::INSTALLED_MODS | CacheSaveCommand::WITH_VERSIONS, this);
    connect(command, &CacheSaveCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheSaveCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::saveCacheSpec()
{
    actionStarted();
    CacheSaveCommand *command = new CacheSaveCommand(app, CacheSaveCommand::CACHED_MODS, this);
    connect(command, &CacheSaveCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheSaveCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::openSettings()
{
    actionStarted();
    SettingsDialog dialog(app, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        app.cache().refresh(ModCache::LATEST_ONLY);
        app.modList().refresh();
        logDisplay->appendPlainText("\n--");
        QString message = QString("Settings updated.  \tInstalled: %1 mods  \tCache: %2 mods").arg(app.modList().mods().size()).arg(app.cache().mods().size());
        logDisplay->appendPlainText(message);
    }
    actionFinished();
}

// Cache actions.

void MainWindow::cacheStatus()
{
    logDisplay->appendPlainText("\n--");
    actionStarted();
    CacheStatusCommand *command = new CacheStatusCommand(app, this);
    connect(command, &CacheStatusCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheStatusCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::cacheUpdate()
{
    logDisplay->appendPlainText("\n--");
    actionStarted();
    CacheUpdateCommand *command = new CacheUpdateCommand(app, this);
    connect(command, &CacheUpdateCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheUpdateCommand::finished, this, &MainWindow::actionFinished);
    connect(command, &CacheUpdateCommand::beginProgress, this, &MainWindow::beginProgress);
    connect(command, &CacheUpdateCommand::updateProgress, this->logProgress, &QProgressBar::setValue);
    command->execute();
}

void MainWindow::cacheAddMod()
{
    actionStarted();
    CacheAddCommand *command = new CacheAddCommand(app, this);
    connect(command, &CacheAddCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheAddCommand::finished, this, &MainWindow::actionFinished);
    connect(command, &CacheAddCommand::beginProgress, this, &MainWindow::beginProgress);
    connect(command, &CacheAddCommand::updateProgress, this->logProgress, &QProgressBar::setValue);
    command->execute();
}

// Installed actions.

void MainWindow::installedStatus()
{
    logDisplay->appendPlainText("\n--");
    actionStarted();
    InstalledStatusCommand *command = new InstalledStatusCommand(app, this);
    connect(command, &InstalledStatusCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledStatusCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::installedUpdate()
{
    logDisplay->appendPlainText("\n--");
    actionStarted();
    InstalledUpdateCommand *command = new InstalledUpdateCommand(app, this);
    connect(command, &InstalledUpdateCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledUpdateCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

} // namespace iimodmanager
