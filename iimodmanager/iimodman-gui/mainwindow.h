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
    void writeText(QString value);

    void cacheStatus();
    void cacheUpdate();
    void cacheAddMod();
    void installedStatus();
    void installedUpdate();

private:
    void createActions();
    void createMenus();
    void disableActions();

    ModManGuiApplication &app;
    QMenu *fileMenu;
    QAction *quitAct;
    QMenu *cacheMenu;
    QAction *cacheStatusAct;
    QAction *cacheUpdateAct;
    QAction *cacheAddModAct;
    QMenu *installedMenu;
    QAction *installedStatusAct;
    QAction *installedUpdateAct;
    QPlainTextEdit *textDisplay;
};

} // namespace iimodmanager

#endif // MAINWINDOW_H

