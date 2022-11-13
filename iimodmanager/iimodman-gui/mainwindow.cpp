#include "cacheaddcommand.h"
#include "cachesavecommand.h"
#include "cachestatuscommand.h"
#include "cacheupdatecommand.h"
#include "installedstatuscommand.h"
#include "installedupdatecommand.h"
#include "installedsyncfilecommand.h"
#include "mainwindow.h"
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
    modsModel = new ModsModel(app.cache(), app.modList());
    modsSortFilterProxy = new ModsSortFilterProxyModel;
    modsSortFilterProxy->setSourceModel(modsModel);
    modsSortFilterProxy->setFilterKeyColumn(0);
    modsSortFilterProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    modsSortFilterProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    modsView = new QTreeView;
    modsView->setModel(modsSortFilterProxy);
    modsView->header()->setStretchLastSection(false);
    modsView->setColumnWidth(ModsModel::NAME, 300);
    modsView->setColumnWidth(ModsModel::ID, 160);
    modsView->setColumnWidth(ModsModel::INSTALLED_VERSION, 110);
    modsView->setColumnWidth(ModsModel::LATEST_VERSION, 110);
    modsView->setColumnWidth(ModsModel::INSTALLED_UPDATE_TIME, 160);
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
    settingsAct = new QAction(tr("&Preferences"), this);
    settingsAct->setShortcuts(QKeySequence::Preferences);
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);
    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    connect(quitAct, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    cacheStatusAct = new QAction(tr("&Log Status"), this);
    cacheStatusAct->setStatusTip(tr("List downloaded mods to the log."));
    connect(cacheStatusAct, &QAction::triggered, this, &MainWindow::cacheStatus);
    cacheUpdateAct = new QAction(tr("Download &updates"), this);
    cacheUpdateAct->setStatusTip(tr("Check for and download updates for all mods in the cache"));
    connect(cacheUpdateAct, &QAction::triggered, this, &MainWindow::cacheUpdate);
    cacheSaveAct = new QAction(tr("&Save all-mods file"), this);
    cacheSaveAct->setStatusTip(tr("Write a modspec file listing all cached mods"));
    connect(cacheSaveAct, &QAction::triggered, this, &MainWindow::cacheSave);
    cacheAddModAct = new QAction(tr("&Add Workshop Mod"), this);
    cacheAddModAct->setStatusTip(tr("Download and add a mod to the cache"));
    connect(cacheAddModAct, &QAction::triggered, this, &MainWindow::cacheAddMod);

    installedStatusAct = new QAction(tr("&Log Status"), this);
    installedStatusAct->setStatusTip(tr("List currently installed mods to the log."));
    connect(installedStatusAct, &QAction::triggered, this, &MainWindow::installedStatus);
    installedUpdateAct = new QAction(tr("Install &Updates"), this);
    installedUpdateAct->setStatusTip(tr("Updates currently installed mods to their latest downloaded version."));
    connect(installedUpdateAct, &QAction::triggered, this, &MainWindow::installedUpdate);
    installedSyncFileAct = new QAction(tr("Sync from &File"), this);
    installedSyncFileAct->setStatusTip(tr("Installs/Updates/Uninstalls mods to match a modspec file."));
    connect(installedSyncFileAct, &QAction::triggered, this, &MainWindow::installedSyncFile);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(settingsAct);
    fileMenu->addAction(quitAct);
    QMenu *cacheMenu = menuBar()->addMenu(tr("&Downloaded"));
    cacheMenu->addAction(cacheStatusAct);
    cacheMenu->addAction(cacheUpdateAct);
    cacheMenu->addAction(cacheSaveAct);
    cacheMenu->addSeparator();
    cacheMenu->addAction(cacheAddModAct);
    QMenu *installedMenu = menuBar()->addMenu(tr("&Installed"));
    installedMenu->addAction(installedStatusAct);
    installedMenu->addAction(installedUpdateAct);
    installedMenu->addSeparator();
    installedMenu->addAction(installedSyncFileAct);
}

void MainWindow::setActionsEnabled(bool enabled)
{
    settingsAct->setEnabled(enabled);
    cacheStatusAct->setEnabled(enabled);
    cacheUpdateAct->setEnabled(enabled);
    cacheSaveAct->setEnabled(enabled);
    cacheAddModAct->setEnabled(enabled);
    installedStatusAct->setEnabled(enabled);
    installedUpdateAct->setEnabled(enabled);
    installedSyncFileAct->setEnabled(enabled);
}

void MainWindow::modsFilterStatusChanged()
{
    ModsModel::Status requiredStatuses, maskedStatuses;

    switch (modsInstalledCheckBox->checkState())
    {
    case Qt::Unchecked:
        maskedStatuses |= ModsModel::INSTALLED_STATUS;
        break;
    case Qt::Checked:
        requiredStatuses |= ModsModel::INSTALLED_STATUS;
        break;
    case Qt::PartiallyChecked:
        break;
    }

    if (modsHasCachedUpdateCheckBox->isChecked())
        requiredStatuses |= ModsModel::CAN_INSTALL_UPDATE_STATUS;

    modsSortFilterProxy->setFilterStatus(requiredStatuses, maskedStatuses);
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

void MainWindow::openSettings()
{
    SettingsDialog dialog(app, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        app.cache().refresh(ModCache::LATEST_ONLY);
        app.modList().refresh();
        logDisplay->appendPlainText("\n--");
        QString message = QString("Settings updated.  \tInstalled: %1 mods  \tCache: %2 mods").arg(app.modList().mods().size()).arg(app.cache().mods().size());
        logDisplay->appendPlainText(message);
    }
}

void MainWindow::cacheStatus()
{
    logDisplay->appendPlainText("\n--");
    CacheStatusCommand *command = new CacheStatusCommand(app, this);
    setActionsEnabled(false);
    connect(command, &CacheStatusCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheStatusCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::cacheUpdate()
{
    logDisplay->appendPlainText("\n--");
    CacheUpdateCommand *command = new CacheUpdateCommand(app, this);
    setActionsEnabled(false);
    connect(command, &CacheUpdateCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheUpdateCommand::finished, this, &MainWindow::actionFinished);
    connect(command, &CacheUpdateCommand::beginProgress, this, &MainWindow::beginProgress);
    connect(command, &CacheUpdateCommand::updateProgress, this->logProgress, &QProgressBar::setValue);
    command->execute();
}

void MainWindow::cacheSave()
{
    CacheSaveCommand *command = new CacheSaveCommand(app, this);
    setActionsEnabled(false);
    connect(command, &CacheSaveCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheSaveCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::cacheAddMod()
{
    CacheAddCommand *command = new CacheAddCommand(app, this);
    setActionsEnabled(false);
    connect(command, &CacheAddCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheAddCommand::finished, this, &MainWindow::actionFinished);
    connect(command, &CacheAddCommand::beginProgress, this, &MainWindow::beginProgress);
    connect(command, &CacheAddCommand::updateProgress, this->logProgress, &QProgressBar::setValue);
    command->execute();
}

void MainWindow::installedStatus()
{
    logDisplay->appendPlainText("\n--");
    InstalledStatusCommand *command = new InstalledStatusCommand(app, this);
    setActionsEnabled(false);
    connect(command, &InstalledStatusCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledStatusCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::installedUpdate()
{
    logDisplay->appendPlainText("\n--");
    InstalledUpdateCommand *command = new InstalledUpdateCommand(app, this);
    setActionsEnabled(false);
    connect(command, &InstalledUpdateCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledUpdateCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::installedSyncFile()
{
    InstalledSyncFileCommand *command = new InstalledSyncFileCommand(app, this);
    setActionsEnabled(false);
    connect(command, &InstalledSyncFileCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledSyncFileCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

} // namespace iimodmanager
