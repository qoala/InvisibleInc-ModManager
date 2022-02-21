#include "cachestatuscommand.h"
#include "cacheupdatecommand.h"
#include "installedstatuscommand.h"
#include "installedupdatecommand.h"
#include "mainwindow.h"
#include "modmanguiapplication.h"

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
    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    connect(quitAct, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    cacheStatusAct = new QAction(tr("&Status"), this);
    cacheStatusAct->setStatusTip(tr("List mods in the cache"));
    connect(cacheStatusAct, &QAction::triggered, this, &MainWindow::cacheStatus);
    cacheUpdateAct = new QAction(tr("&Update Cache"), this);
    cacheUpdateAct->setStatusTip(tr("Check for and download updates for all mods in the cache"));
    connect(cacheUpdateAct, &QAction::triggered, this, &MainWindow::cacheUpdate);
    cacheAddModAct = new QAction(tr("&Add Workshop Mod"), this);
    cacheAddModAct->setStatusTip(tr("Download and add a mod to the cache"));
    connect(cacheAddModAct, &QAction::triggered, this, &MainWindow::cacheAddMod);

    installedStatusAct = new QAction(tr("&Status"), this);
    installedStatusAct->setStatusTip(tr("List currently installed mods"));
    connect(installedStatusAct, &QAction::triggered, this, &MainWindow::installedStatus);
    installedUpdateAct = new QAction(tr("Install &Updates"), this);
    installedUpdateAct->setStatusTip(tr("Updates currently installed mods to the latest downloaded version."));
    connect(installedUpdateAct, &QAction::triggered, this, &MainWindow::installedUpdate);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(quitAct);
    cacheMenu = menuBar()->addMenu(tr("&Cache"));
    cacheMenu->addAction(cacheStatusAct);
    cacheMenu->addAction(cacheUpdateAct);
    cacheMenu->addSeparator();
    cacheMenu->addAction(cacheAddModAct);
    installedMenu = menuBar()->addMenu(tr("&Installed"));
    installedMenu->addAction(installedStatusAct);
    installedMenu->addAction(installedUpdateAct);
}

void MainWindow::disableActions()
{
    cacheStatusAct->setEnabled(false);
    cacheUpdateAct->setEnabled(false);
    cacheAddModAct->setEnabled(false);
    installedStatusAct->setEnabled(false);
    installedUpdateAct->setEnabled(false);
}

void MainWindow::enableActions()
{
    cacheStatusAct->setEnabled(true);
    cacheUpdateAct->setEnabled(true);
    cacheAddModAct->setEnabled(true);
    installedStatusAct->setEnabled(true);
    installedUpdateAct->setEnabled(true);
}

void MainWindow::writeText(QString value)
{
    textDisplay->appendPlainText(value);
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

} // namespace iimodmanager
