#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>

namespace iimodmanager {

class ModManGuiApplication;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(ModManGuiApplication &app, QWidget *parent = nullptr);

private slots:
    void resetSettings();
    void applySettings();

    void browseCachePath();
    void browseInstallPath();

private:
    QLineEdit *cachePathLine;
    QAction* cachePathBrowseAct;
    QLineEdit *installPathLine;
    QAction* installPathBrowseAct;
    QDialogButtonBox *buttonBox;
    QPushButton *resetButton;
    QPushButton *cancelButton;
    QPushButton *okButton;

    ModManGuiApplication &app;

    void loadSettings();
};

} // namespace iimodmanager

#endif // SETTINGSDIALOG_H
