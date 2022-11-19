#include "applypreviewcommand.h"
#include "cacheaddcommand.h"
#include "cacheimportinstalledcommand.h"
#include "cachesavecommand.h"
#include "cacheupdatecommand.h"
#include "mainwindow.h"
#include "markupdatescommand.h"
#include "modelutil.h"
#include "modmanguiapplication.h"
#include "modspecpreviewmodel.h"
#include "modssortfilterproxymodel.h"
#include "openspecfilecommand.h"
#include "settingsdialog.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QShortcut>
#include <QVBoxLayout>
#include <modcache.h>
#include <modlist.h>
#include <modspec.h>

#ifndef IIMODMANVERSION
#  define IIMODMANVERSION ""
#endif

namespace iimodmanager {

MainWindow::MainWindow(ModManGuiApplication &app)
    : app(app), isLocked_(false)
{
    createTabs();
    createLogDock();
    createMenuActions();

    setWindowTitle(tr("II Mod Manager %1").arg(IIMODMAN_VERSION));
    resize(980, 460);
}

void MainWindow::createTabs()
{
    tabWidget = new QTabWidget;
    setCentralWidget(tabWidget);

    // Main mods tab.
    modsPreviewModel = new ModSpecPreviewModel(app.cache(), app.modList(), this);
    connect(modsPreviewModel, &ModSpecPreviewModel::textOutput, this, &MainWindow::writeText);
    connect(this, &MainWindow::lockChanged, modsPreviewModel, &ModSpecPreviewModel::setLock);
    modsSortFilterProxy = new ModsSortFilterProxyModel(this);
    modsSortFilterProxy->setSourceModel(modsPreviewModel);
    modsSortFilterProxy->setFilterTextColumns({ModSpecPreviewModel::NAME, ModSpecPreviewModel::ID});
    modsSortFilterProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    modsSortFilterProxy->setFilterStatusColumn(ModSpecPreviewModel::NAME);
    modsSortFilterProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    modsView = new QTreeView;
    modsView->setModel(modsSortFilterProxy);
    modsView->header()->setStretchLastSection(false);
    modsView->setColumnWidth(ModSpecPreviewModel::NAME, 300);
    modsView->setColumnWidth(ModSpecPreviewModel::ID, 160);
    modsView->setColumnWidth(ModSpecPreviewModel::INSTALLED_VERSION, 110);
    modsView->setColumnWidth(ModSpecPreviewModel::LATEST_VERSION, 110);
    modsView->setColumnWidth(ModSpecPreviewModel::TARGET_VERSION, 110);
    modsView->setColumnWidth(ModSpecPreviewModel::INSTALLED_VERSION_TIME, 160);
    modsView->setColumnWidth(ModSpecPreviewModel::CACHE_UPDATE_TIME, 160);
    modsView->setColumnWidth(ModSpecPreviewModel::TARGET_VERSION_TIME, 160);
    modsView->sortByColumn(ModSpecPreviewModel::NAME, Qt::AscendingOrder);
    modsView->setSortingEnabled(true);

    modsSearchInput = new QLineEdit;
    connect(modsSearchInput, &QLineEdit::textChanged, modsSortFilterProxy, &QSortFilterProxyModel::setFilterFixedString);
    QShortcut *modsSearchShortcut = new QShortcut(QKeySequence::Find, modsSearchInput);
    connect(modsSearchShortcut, &QShortcut::activated,
            modsSearchInput, [=](){ this->modsSearchInput->setFocus(Qt::ShortcutFocusReason); });

    modsInstalledCheckBox = new QCheckBox(tr("Installed"));
    connect(modsInstalledCheckBox, &QCheckBox::stateChanged, this, &MainWindow::modsFilterStatusChanged);
    modsHasCachedUpdateCheckBox = new QCheckBox(tr("Update Ready"));
    modsHasCachedUpdateCheckBox->setToolTip(tr("A new version is downloaded and ready to install."));
    connect(modsHasCachedUpdateCheckBox, &QCheckBox::stateChanged, this, &MainWindow::modsFilterStatusChanged);

    QDialogButtonBox *modsActionBtnBox = new QDialogButtonBox();
    applyPreviewBtn = modsActionBtnBox->addButton(tr("Apply Changes"), QDialogButtonBox::ApplyRole);
    applyPreviewBtn->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Return));
    applyPreviewBtn->setEnabled(false);
    connect(applyPreviewBtn, &QAbstractButton::clicked, this, &MainWindow::applyPreview);
    revertPreviewBtn = modsActionBtnBox->addButton(tr("Reset Changes"), QDialogButtonBox::ResetRole);
    revertPreviewBtn->setEnabled(false);
    connect(revertPreviewBtn, &QAbstractButton::clicked, this, &MainWindow::revertPreview);
    connect(modsPreviewModel, &ModSpecPreviewModel::isEmptyChanged, this, &MainWindow::updatePreviewActionsEnabled);

    QHBoxLayout *modsFilterBtnLayout = new QHBoxLayout;
    modsFilterBtnLayout->addWidget(modsInstalledCheckBox);
    modsFilterBtnLayout->addWidget(modsHasCachedUpdateCheckBox);
    QVBoxLayout *modsLayout = new QVBoxLayout;
    modsLayout->addWidget(modsSearchInput);
    modsLayout->addLayout(modsFilterBtnLayout);
    modsLayout->addWidget(modsView);
    modsLayout->addWidget(modsActionBtnBox);
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

    QDockWidget *logDock = new QDockWidget(tr("Output"), this);
    logDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    logDock->setFeatures(QDockWidget::DockWidgetMovable); // Not floatable, Not closable.
    logDock->setWidget(dockContents);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

void MainWindow::createMenuActions()
{
    openSpecAct = new QAction(tr("&Open Spec File..."), this);
    openSpecAct->setStatusTip(tr("Update the proposed mods to the selected file's specification."));
    openSpecAct->setShortcuts(QKeySequence::Open);
    connect(openSpecAct, &QAction::triggered, this, &MainWindow::openSpec);
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

    cacheUpdateAct = new QAction(tr("Download &Updates"), this);
    cacheUpdateAct->setStatusTip(tr("Check for and download updates for all mods in the cache"));
    connect(cacheUpdateAct, &QAction::triggered, this, &MainWindow::cacheUpdate);
    markUpdatesAct = new QAction(tr("Mark &Updates For Installation"), this);
    markUpdatesAct->setStatusTip(tr("Propose updating all installed mods with newer downloaded versions. Click \"Apply\" to install."));
    connect(markUpdatesAct, &QAction::triggered, this, &MainWindow::markUpdates);
    cacheAddModAct = new QAction(tr("&Add Workshop Mod"), this);
    cacheAddModAct->setStatusTip(tr("Download and add a mod to the cache"));
    connect(cacheAddModAct, &QAction::triggered, this, &MainWindow::cacheAddMod);
    cacheImportInstalledAct = new QAction(tr("&Import installed mods"), this);
    cacheImportInstalledAct->setStatusTip(tr("Copy existing installed mods to the download cache"));
    connect(cacheImportInstalledAct, &QAction::triggered, this, &MainWindow::cacheImportInstalled);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openSpecAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveInstalledSpecAct);
    fileMenu->addAction(saveInstalledVersionSpecAct);
    fileMenu->addAction(saveCacheSpecAct);
    fileMenu->addSeparator();
    fileMenu->addAction(settingsAct);
    fileMenu->addAction(quitAct);
    QMenu *cacheMenu = menuBar()->addMenu(tr("&Cache"));
    cacheMenu->addAction(cacheUpdateAct);
    cacheMenu->addAction(markUpdatesAct);
    cacheMenu->addSeparator();
    cacheMenu->addAction(cacheAddModAct);
    cacheMenu->addAction(cacheImportInstalledAct);
}

void MainWindow::setActionsEnabled(bool enabled)
{
    updatePreviewActionsEnabled();

    openSpecAct->setEnabled(enabled);
    saveInstalledSpecAct->setEnabled(enabled);
    saveInstalledVersionSpecAct->setEnabled(enabled);
    saveCacheSpecAct->setEnabled(enabled);
    settingsAct->setEnabled(enabled);
    cacheUpdateAct->setEnabled(enabled);
    markUpdatesAct->setEnabled(enabled);
    cacheAddModAct->setEnabled(enabled);
    cacheImportInstalledAct->setEnabled(enabled);
}

void MainWindow::updatePreviewActionsEnabled()
{
    bool enabled = !isLocked() && !modsPreviewModel->isEmpty();
    applyPreviewBtn->setEnabled(enabled);
    revertPreviewBtn->setEnabled(enabled);
}

void MainWindow::actionStarted()
{
    setLock(true);
}

void MainWindow::actionFinished()
{
    setLock(false);
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

void MainWindow::openSpec()
{
    OpenSpecFileCommand *command = new OpenSpecFileCommand(app, this);
    connect(command, &OpenSpecFileCommand::started, this, &MainWindow::actionStarted);
    connect(command, &OpenSpecFileCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &OpenSpecFileCommand::modSpecReady, modsPreviewModel, &ModSpecPreviewModel::setModSpec);
    connect(command, &OpenSpecFileCommand::finished, this, &MainWindow::actionFinished);
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

void MainWindow::markUpdates()
{
    actionStarted();
    MarkUpdatesCommand *command = new MarkUpdatesCommand(app, modsPreviewModel, this);
    connect(command, &MarkUpdatesCommand::finished, this, &MainWindow::actionFinished);
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

void MainWindow::cacheImportInstalled()
{
    logDisplay->appendPlainText("\n--");
    actionStarted();
    CacheImportInstalledCommand *command = new CacheImportInstalledCommand(app, this);
    connect(command, &CacheImportInstalledCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheImportInstalledCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

// Main mods page.

void MainWindow::modsFilterStatusChanged()
{
    modelutil::Status requiredStatuses, maskedStatuses;

    if (modsInstalledCheckBox->isChecked())
        requiredStatuses |= modelutil::INSTALLED_STATUS;

    if (modsHasCachedUpdateCheckBox->isChecked())
        requiredStatuses |= modelutil::CAN_INSTALL_UPDATE_STATUS;

    modsSortFilterProxy->setFilterStatus(requiredStatuses, maskedStatuses);
}

void MainWindow::applyPreview()
{
    logDisplay->appendPlainText("\n--");
    actionStarted();
    ApplyPreviewCommand *command = new ApplyPreviewCommand(app, modsPreviewModel, this);
    connect(command, &ApplyPreviewCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &ApplyPreviewCommand::finished, this, &MainWindow::actionFinished);
    command->execute();
}

void MainWindow::revertPreview()
{
    modsPreviewModel->revert();
    updatePreviewActionsEnabled();
}

} // namespace iimodmanager
