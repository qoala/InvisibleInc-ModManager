#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>

namespace iimodmanager {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void addMod();

private:
    void createActions();
    void createMenus();

    QMenu *fileMenu;
    QAction *quitAct;
    QMenu *cacheMenu;
    QAction *addModAct;
    QLabel *infoLabel;
};

} // namespace iimodmanager

#endif // MAINWINDOW_H

