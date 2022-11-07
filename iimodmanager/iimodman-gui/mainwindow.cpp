#include "cachesavecommand.h"
#include "cachestatuscommand.h"
#include "cacheupdatecommand.h"
#include "installedstatuscommand.h"
#include "installedupdatecommand.h"
#include "installedsyncfilecommand.h"
#include "mainwindow.h"
#include "modmanguiapplication.h"
#include "settingsdialog.h"

#include <QApplication>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextTable>
#include <QVBoxLayout>

namespace iimodmanager {

MainWindow::MainWindow(ModManGuiApplication &app)
    : app(app)
{
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    QString message = QString("Mod Manager loaded.  \tInstalled: %1 mods  \tCache: %2 mods").arg(app.modList().mods().size()).arg(app.cache().mods().size());
    statusBar()->showMessage(message);

    textDisplay = new QPlainTextEdit(message);
    textDisplay->setReadOnly(true);
    textDisplay->setMaximumBlockCount(10000);
    textDisplay->setTabStopDistance(240);
    textDisplay->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    textDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(textDisplay);
    widget->setLayout(layout);

    createActions();
    createMenus();

    setWindowTitle(tr("II Mod Manager"));
    setMinimumSize(160, 160);
    resize(980, 460);
}

void MainWindow::createActions()
{
    settingsAct = new QAction(tr("&Preferences"), this);
    settingsAct->setShortcuts(QKeySequence::Preferences);
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);
    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    connect(quitAct, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    cacheStatusAct = new QAction(tr("&Status"), this);
    cacheStatusAct->setStatusTip(tr("List mods in the cache"));
    connect(cacheStatusAct, &QAction::triggered, this, &MainWindow::cacheStatus);
    cacheUpdateAct = new QAction(tr("&Update Cache"), this);
    cacheUpdateAct->setStatusTip(tr("Check for and download updates for all mods in the cache"));
    connect(cacheUpdateAct, &QAction::triggered, this, &MainWindow::cacheUpdate);
    cacheSaveAct = new QAction(tr("&Write all-mods file"), this);
    cacheSaveAct->setStatusTip(tr("Write a modspec file listing all cached mods"));
    connect(cacheSaveAct, &QAction::triggered, this, &MainWindow::cacheSave);
    cacheAddModAct = new QAction(tr("&Add Workshop Mod"), this);
    cacheAddModAct->setStatusTip(tr("Download and add a mod to the cache"));
    connect(cacheAddModAct, &QAction::triggered, this, &MainWindow::cacheAddMod);

    installedStatusAct = new QAction(tr("&Status"), this);
    installedStatusAct->setStatusTip(tr("List currently installed mods"));
    connect(installedStatusAct, &QAction::triggered, this, &MainWindow::installedStatus);
    installedUpdateAct = new QAction(tr("Install &Updates"), this);
    installedUpdateAct->setStatusTip(tr("Updates currently installed mods to the latest downloaded version."));
    connect(installedUpdateAct, &QAction::triggered, this, &MainWindow::installedUpdate);
    installedSyncFileAct = new QAction(tr("Sync from &File"), this);
    installedSyncFileAct->setStatusTip(tr("Installs/Updates/Uninstalls mods to match a modspec file."));
    connect(installedSyncFileAct, &QAction::triggered, this, &MainWindow::installedSyncFile);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(settingsAct);
    fileMenu->addAction(quitAct);
    cacheMenu = menuBar()->addMenu(tr("&Cache"));
    cacheMenu->addAction(cacheStatusAct);
    cacheMenu->addAction(cacheUpdateAct);
    cacheMenu->addAction(cacheSaveAct);
    cacheMenu->addSeparator();
    cacheMenu->addAction(cacheAddModAct);
    installedMenu = menuBar()->addMenu(tr("&Installed"));
    installedMenu->addAction(installedStatusAct);
    installedMenu->addAction(installedUpdateAct);
    installedMenu->addSeparator();
    installedMenu->addAction(installedSyncFileAct);
}

void MainWindow::disableActions()
{
    cacheStatusAct->setEnabled(false);
    cacheUpdateAct->setEnabled(false);
    cacheSaveAct->setEnabled(false);
    cacheAddModAct->setEnabled(false);
    installedStatusAct->setEnabled(false);
    installedUpdateAct->setEnabled(false);
    installedSyncFileAct->setEnabled(false);
}

void MainWindow::enableActions()
{
    cacheStatusAct->setEnabled(true);
    cacheUpdateAct->setEnabled(true);
    cacheSaveAct->setEnabled(true);
    cacheAddModAct->setEnabled(true);
    installedStatusAct->setEnabled(true);
    installedUpdateAct->setEnabled(true);
    installedSyncFileAct->setEnabled(true);
}

void MainWindow::writeText(QString value)
{
    textDisplay->appendPlainText(value);
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(app, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        app.cache().refresh(ModCache::LATEST_ONLY);
        app.modList().refresh();
        textDisplay->appendPlainText("\n--");
        QString message = QString("Settings updated.  \tInstalled: %1 mods  \tCache: %2 mods").arg(app.modList().mods().size()).arg(app.cache().mods().size());
        textDisplay->appendPlainText(message);
        statusBar()->showMessage(message);
    }
}

void MainWindow::cacheStatus()
{
    textDisplay->appendPlainText("\n--");
    CacheStatusCommand *command = new CacheStatusCommand(app, this);
    disableActions();
    connect(command, &CacheStatusCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheStatusCommand::finished, this, &MainWindow::enableActions);
    command->execute();
}

void MainWindow::cacheUpdate()
{
    textDisplay->appendPlainText("\n--");
    CacheUpdateCommand *command = new CacheUpdateCommand(app, this);
    disableActions();
    connect(command, &CacheUpdateCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheUpdateCommand::finished, this, &MainWindow::enableActions);
    command->execute();
}

void MainWindow::cacheSave()
{
    CacheSaveCommand *command = new CacheSaveCommand(app, this);
    disableActions();
    connect(command, &CacheSaveCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &CacheSaveCommand::finished, this, &MainWindow::enableActions);
    command->execute();
}

void MainWindow::cacheAddMod()
{}

void MainWindow::installedStatus()
{
    textDisplay->appendPlainText("\n--");
    InstalledStatusCommand *command = new InstalledStatusCommand(app, this);
    disableActions();
    connect(command, &InstalledStatusCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledStatusCommand::finished, this, &MainWindow::enableActions);
    command->execute();
}

void MainWindow::installedUpdate()
{
    textDisplay->appendPlainText("\n--");
    InstalledUpdateCommand *command = new InstalledUpdateCommand(app, this);
    disableActions();
    connect(command, &InstalledUpdateCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledUpdateCommand::finished, this, &MainWindow::enableActions);
    command->execute();
}

void MainWindow::installedSyncFile()
{
    InstalledSyncFileCommand *command = new InstalledSyncFileCommand(app, this);
    disableActions();
    connect(command, &InstalledSyncFileCommand::textOutput, this, &MainWindow::writeText);
    connect(command, &InstalledSyncFileCommand::finished, this, &MainWindow::enableActions);
    command->execute();
}

} // namespace iimodmanager
