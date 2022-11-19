#include "settingsdialog.h"
#include "modmanguiapplication.h"

#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QIcon>
#include <QStyle>
#include <QVBoxLayout>

namespace iimodmanager {

SettingsDialog::SettingsDialog(ModManGuiApplication  &app, QWidget *parent)
    : QDialog(parent), app(app)
{
    cachePathLine = new QLineEdit;
    cachePathBrowseAct = cachePathLine->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), QLineEdit::TrailingPosition);
    connect(cachePathBrowseAct, &QAction::triggered, this, &SettingsDialog::browseCachePath);
    installPathLine = new QLineEdit;
    installPathBrowseAct = installPathLine->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), QLineEdit::TrailingPosition);
    connect(installPathBrowseAct, &QAction::triggered, this, &SettingsDialog::browseInstallPath);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Invisible Inc Install Path"), installPathLine);
    formLayout->addRow(tr("Mod-Cache Path"), cachePathLine);

    buttonBox = new QDialogButtonBox();
    resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
    connect(resetButton, &QAbstractButton::clicked, this, &SettingsDialog::resetSettings);
    cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
    okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
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
    ModManConfig &config = app.config();
    cachePathLine->setText(QDir::toNativeSeparators(config.cachePath()));
    installPathLine->setText(QDir::toNativeSeparators(config.installPath()));
}

void SettingsDialog::resetSettings()
{
    ModManConfig &config = app.config();
    cachePathLine->setText(QDir::toNativeSeparators(config.defaultCachePath()));
    installPathLine->setText(QDir::toNativeSeparators(config.defaultInstallPath()));
}

void SettingsDialog::applySettings()
{
    ModManConfig &config = app.config();
    config.setCachePath(QDir::fromNativeSeparators(cachePathLine->text()));
    config.setInstallPath(QDir::fromNativeSeparators(installPathLine->text()));
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

