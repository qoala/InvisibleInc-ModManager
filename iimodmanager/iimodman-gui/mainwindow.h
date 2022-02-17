#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QLabel>
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
    void addMod();

private:
    void createActions();
    void createMenus();

    ModManGuiApplication &app;
    QMenu *fileMenu;
    QAction *quitAct;
    QMenu *cacheMenu;
    QAction *addModAct;
    QLabel *infoLabel;
};

} // namespace iimodmanager

#endif // MAINWINDOW_H

