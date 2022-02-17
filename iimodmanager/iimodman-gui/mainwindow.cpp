#include "mainwindow.h"
#include "modmanguiapplication.h"

#include <QApplication>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextCursor>
#include <QTextTable>
#include <QVBoxLayout>
#include <modinfo.h>

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

    installedStatusAct = new QAction(tr("&Status"), this);
    installedStatusAct->setStatusTip(tr("List currently installed mods"));
    connect(installedStatusAct, &QAction::triggered, this, &MainWindow::installedStatus);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(quitAct);
    cacheMenu = menuBar()->addMenu(tr("&Cache"));
    cacheMenu->addAction(addModAct);
    installedMenu = menuBar()->addMenu(tr("&Installed"));
    installedMenu->addAction(installedStatusAct);
}

void MainWindow::addMod()
{}

bool compareModNames(const InstalledMod &a, const InstalledMod &b)
{
    return a.info().name() < b.info().name();
}

void MainWindow::installedStatus()
{
    QTextCursor cursor = textDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n\n---\nCurrently Installed Mods\n\n");

    QList<InstalledMod> mods(app.modList().mods());
    std::sort(mods.begin(), mods.end(), compareModNames);
    for (auto mod : mods) {
        const ModInfo &info = mod.info();

        cursor.insertText(info.name());
        cursor.insertText("\t");
        cursor.insertText(info.id());
        cursor.insertText("\t");
        cursor.insertText(mod.versionString());
        if (!mod.hasCacheVersion())
        {
            cursor.insertText("\t");
            cursor.insertText("(not in cache)");
        }
        cursor.insertBlock();
    }
}

} // namespace iimodmanager
