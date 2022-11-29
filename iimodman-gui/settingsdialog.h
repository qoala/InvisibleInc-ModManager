#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
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
    void cachePathChanged();
    void installPathChanged();

private:
    ModManGuiApplication &app;

    // General
    QLineEdit *cachePathLine;
    QAction* cachePathBrowseAct;
    QLabel *cachePathValidationLabel;
    QLineEdit *installPathLine;
    QAction* installPathBrowseAct;
    QLabel *installPathValidationLabel;

    // Appearance
    QCheckBox *openMaximizedCheckBox;

    // Buttons
    QPushButton *resetButton;
    QPushButton *cancelButton;
    QPushButton *okButton;

    void loadSettings();
};

} // namespace iimodmanager

#endif // SETTINGSDIALOG_H
