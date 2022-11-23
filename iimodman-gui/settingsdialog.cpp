#include "guiconfig.h"
#include "settingsdialog.h"
#include "modmanguiapplication.h"

#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QIcon>
#include <QStyle>
#include <QTabWidget>
#include <QVBoxLayout>

namespace iimodmanager {

SettingsDialog::SettingsDialog(ModManGuiApplication  &app, QWidget *parent)
    : QDialog(parent), app(app)
{
    QTabWidget *tabWidget = new QTabWidget;

    // General settings.

    cachePathLine = new QLineEdit;
    cachePathBrowseAct = cachePathLine->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), QLineEdit::TrailingPosition);
    connect(cachePathBrowseAct, &QAction::triggered, this, &SettingsDialog::browseCachePath);
    installPathLine = new QLineEdit;
    installPathBrowseAct = installPathLine->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), QLineEdit::TrailingPosition);
    connect(installPathBrowseAct, &QAction::triggered, this, &SettingsDialog::browseInstallPath);

    QFormLayout *generalLayout = new QFormLayout;
    generalLayout->addRow(tr("Invisible Inc Install Path"), installPathLine);
    generalLayout->addRow(tr("Mod-Cache Path"), cachePathLine);
    QWidget *generalPage = new QWidget;
    generalPage->setLayout(generalLayout);
    tabWidget->addTab(generalPage, tr("General"));

    // Appearance settings.

    openMaximizedCheckBox = new QCheckBox(tr("Open windows/dialogs maximized"));

    QFormLayout *appearanceLayout = new QFormLayout;
    appearanceLayout->addRow(openMaximizedCheckBox);
    QWidget *appearancePage = new QWidget;
    appearancePage->setLayout(appearanceLayout);
    tabWidget->addTab(appearancePage, tr("Appearance"));

    // Buttons.

    buttonBox = new QDialogButtonBox();
    resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
    connect(resetButton, &QAbstractButton::clicked, this, &SettingsDialog::resetSettings);
    cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
    okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    loadSettings();
    connect(this, &QDialog::accepted, this, &SettingsDialog::applySettings);

    setWindowTitle(tr("II Mod Manager: Preferences"));
    setSizeGripEnabled(true);
    resize(800, 0);
}

void SettingsDialog::loadSettings()
{
    const GuiConfig &config = app.config();
    cachePathLine->setText(QDir::toNativeSeparators(config.cachePath()));
    installPathLine->setText(QDir::toNativeSeparators(config.installPath()));

    openMaximizedCheckBox->setCheckState(config.openMaximized() ? Qt::Checked : Qt::Unchecked);
}

void SettingsDialog::resetSettings()
{
    const GuiConfig &config = app.config();
    cachePathLine->setText(QDir::toNativeSeparators(config.defaultCachePath()));
    installPathLine->setText(QDir::toNativeSeparators(config.defaultInstallPath()));

    openMaximizedCheckBox->setCheckState(Qt::Unchecked);
}

void SettingsDialog::applySettings()
{
    GuiConfig &config = app.mutableConfig();

    config.setCachePath(QDir::fromNativeSeparators(cachePathLine->text()));
    config.setInstallPath(QDir::fromNativeSeparators(installPathLine->text()));

    config.setOpenMaximized(openMaximizedCheckBox->isChecked());
}

void SettingsDialog::browseCachePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Cache Path"), QDir::fromNativeSeparators(cachePathLine->text()));
    if (!dir.isEmpty())
    {
        cachePathLine->setText(QDir::toNativeSeparators(dir));
    }
}

void SettingsDialog::browseInstallPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Cache Path"), QDir::fromNativeSeparators(installPathLine->text()));
    if (!dir.isEmpty())
    {
        installPathLine->setText(QDir::toNativeSeparators(dir));
    }
}

} // namespace iimodmanager

