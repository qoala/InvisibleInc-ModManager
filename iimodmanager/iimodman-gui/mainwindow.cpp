#include "mainwindow.h"
#include "modmanguiapplication.h"

#include <QApplication>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>

namespace iimodmanager {

MainWindow::MainWindow(ModManGuiApplication &app)
    : app(app)
{
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    QString message = QString("Mod Manager loaded.  \tInstalled: %1 mods  \tCache: %2 mods").arg(app.modList().mods().size()).arg(app.cache().mods().size());
    statusBar()->showMessage(message);

    infoLabel = new QLabel(message);
    infoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    infoLabel->setAlignment(Qt::AlignLeft);
    infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(infoLabel);
    widget->setLayout(layout);

    createActions();
    createMenus();

    setWindowTitle(tr("II Mod Manager"));
    setMinimumSize(160, 160);
    resize(480, 320);
}

void MainWindow::createActions()
{
    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    connect(quitAct, &QAction::triggered, QApplication::instance(), &QApplication::quit);
    addModAct = new QAction(tr("&Add Workshop Mod"), this);
    addModAct->setStatusTip(tr("Download and add a mod to the cache"));
    connect(addModAct, &QAction::triggered, this, &MainWindow::addMod);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(quitAct);
    cacheMenu = menuBar()->addMenu(tr("&Cache"));
    cacheMenu->addAction(addModAct);
}

void MainWindow::addMod()
{}

} // namespace iimodmanager
