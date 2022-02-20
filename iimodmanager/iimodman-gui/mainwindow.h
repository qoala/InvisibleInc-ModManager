#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QPlainTextEdit>
#include <QMainWindow>
#include <QMenu>

namespace iimodmanager {

class ModManGuiApplication;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ModManGuiApplication &app);

private slots:
    void enableActions();
    void cacheUpdate();
    void cacheAddMod();
    void installedStatus();

private:
    void createActions();
    void createMenus();
    void disableActions();

    ModManGuiApplication &app;
    QMenu *fileMenu;
    QAction *quitAct;
    QMenu *cacheMenu;
    QAction *cacheUpdateAct;
    QAction *cacheAddModAct;
    QMenu *installedMenu;
    QAction *installedStatusAct;
    QPlainTextEdit *textDisplay;
};

} // namespace iimodmanager

#endif // MAINWINDOW_H

