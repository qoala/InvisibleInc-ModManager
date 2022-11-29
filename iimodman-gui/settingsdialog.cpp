#include "guiconfig.h"
#include "settingsdialog.h"
#include "modmanguiapplication.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QTabWidget>
#include <QVBoxLayout>

namespace iimodmanager {

SettingsDialog::SettingsDialog(ModManGuiApplication  &app, QWidget *parent)
    : QDialog(parent), app(app)
{
    QTabWidget *tabWidget = new QTabWidget;

    // General settings.

    cachePathValidationLabel = new QLabel;
    cachePathValidationLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    cachePathLine = new QLineEdit;
    connect(cachePathLine, &QLineEdit::editingFinished, this, &SettingsDialog::cachePathChanged);
    cachePathBrowseAct = cachePathLine->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), QLineEdit::TrailingPosition);
    connect(cachePathBrowseAct, &QAction::triggered, this, &SettingsDialog::browseCachePath);

    installPathValidationLabel = new QLabel;
    installPathValidationLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    installPathLine = new QLineEdit;
    connect(installPathLine, &QLineEdit::editingFinished, this, [=](){ this->installPathChanged(true); });
    installPathBrowseAct = installPathLine->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), QLineEdit::TrailingPosition);
    connect(installPathBrowseAct, &QAction::triggered, this, &SettingsDialog::browseInstallPath);

    QFormLayout *generalLayout = new QFormLayout;
    generalLayout->addRow(tr("Invisible Inc Install Path"), installPathLine);
    generalLayout->addRow(QString(), installPathValidationLabel);
    generalLayout->addRow(tr("Mod-Cache Path"), cachePathLine);
    generalLayout->addRow(QString(), cachePathValidationLabel);
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

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
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
    resize(900, 0);
}

void SettingsDialog::loadSettings()
{
    const GuiConfig &config = app.config();
    cachePathLine->setText(QDir::toNativeSeparators(config.cachePath()));
    installPathLine->setText(QDir::toNativeSeparators(config.installPath()));

    openMaximizedCheckBox->setCheckState(config.openMaximized() ? Qt::Checked : Qt::Unchecked);

    cachePathChanged();
    installPathChanged(false);
}

void SettingsDialog::resetSettings()
{
    const GuiConfig &config = app.config();
    cachePathLine->setText(QDir::toNativeSeparators(config.defaultCachePath()));
    installPathLine->setText(QDir::toNativeSeparators(config.defaultInstallPath()));

    openMaximizedCheckBox->setCheckState(Qt::Unchecked);

    cachePathChanged();
    installPathChanged(false);
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
        cachePathChanged();
    }
}

void SettingsDialog::browseInstallPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Cache Path"), QDir::fromNativeSeparators(installPathLine->text()));
    if (!dir.isEmpty())
    {
        installPathLine->setText(QDir::toNativeSeparators(dir));
        installPathChanged(true);
    }
}

void SettingsDialog::cachePathChanged()
{
    QDir dir(QDir::fromNativeSeparators(cachePathLine->text()));
    if (dir.exists() && dir.exists("modmandb.json"))
    {
        cachePathValidationLabel->setText(tr("Download cache found."));
        cachePathValidationLabel->setStyleSheet("color: green");
    }
    else if (dir.cdUp())
    {
        cachePathValidationLabel->setText(tr("New download cache."));
        cachePathValidationLabel->setStyleSheet("color: green");
    }
    else
    {
        cachePathValidationLabel->setText(tr("Neither cache path nor its parent directory exist."));
        cachePathValidationLabel->setStyleSheet("color: red");
    }
}

void SettingsDialog::installPathChanged(bool isUserInput)
{
    QString installPath = QDir::fromNativeSeparators(installPathLine->text());
    if (ModManConfig::isValidInstallPath(installPath))
    {
        installPathValidationLabel->setText(tr("Invisible Inc found."));
        installPathValidationLabel->setStyleSheet("color: green");

        if (isUserInput)
        {
            // Update cache path if it doesn't point to an existing cache.
            QString cachePath = QDir::fromNativeSeparators(cachePathLine->text());
            QDir cacheDir(cachePath);
            if (!cacheDir.exists() || !cacheDir.exists("modmandb.json"))
            {
                QString proposedCachePath = installPath + "/mods-cache";
                if (proposedCachePath != cachePath)
                {
                    cachePathLine->setText(QDir::toNativeSeparators(proposedCachePath));
                    cachePathChanged();
                }
            }
        }
    }
    else
    {
        installPathValidationLabel->setText(tr("Invisible Inc install not found."));
        installPathValidationLabel->setStyleSheet("color: red");
    }
}

} // namespace iimodmanager

