#include "cacheimportinstalledcommand.h"
#include "cacheimportmodel.h"
#include "guidownloader.h"
#include "modmanguiapplication.h"
#include "modssortfilterproxymodel.h"
#include "util.h"

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <modcache.h>
#include <moddownloader.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

class CacheImportDialog : public QDialog
{
    Q_OBJECT
public:
    CacheImportDialog(CacheImportModel *model, QWidget *parent)
        : QDialog(parent), model(model)
    {
        ModsSortFilterProxyModel *proxy = new ModsSortFilterProxyModel(this);
        proxy->setSourceModel(model);
        proxy->setFilterStatus(modelutil::NO_STATUS, modelutil::NULL_STATUS);
        proxy->setFilterStatusColumn(CacheImportModel::NAME);

        QTreeView *treeView = new QTreeView;
        treeView->setModel(proxy);
        treeView->setColumnWidth(CacheImportModel::ACTION, 160);
        treeView->setColumnWidth(CacheImportModel::ID, 160);
        treeView->setColumnWidth(CacheImportModel::NAME, 300);
        treeView->setColumnWidth(CacheImportModel::FOLDER, 160);
        treeView->setColumnWidth(CacheImportModel::INSTALLED_VERSION, 110);
        treeView->setColumnWidth(CacheImportModel::STEAM_UPDATE_TIME, 160);
        treeView->sortByColumn(CacheImportModel::ACTION, Qt::DescendingOrder);
        treeView->setSortingEnabled(true);

        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        applyButton = buttonBox->addButton(QDialogButtonBox::Apply);
        connect(model, &CacheImportModel::isEmptyChanged, this, &CacheImportDialog::updateButtons);
        connect(applyButton, &QAbstractButton::clicked, this, &QDialog::accept);
        cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
        connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
        updateButtons();

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(treeView);
        mainLayout->addWidget(buttonBox);
        setLayout(mainLayout);

        setWindowTitle(tr("II Mod Manager: Import Mods"));
        setSizeGripEnabled(true);
        resize(1080, 400);
    }

private slots:
    void updateButtons() { applyButton->setEnabled(!model->isEmpty()); }

private:
    CacheImportModel *model;
    QPushButton *applyButton;
    QPushButton *cancelButton;
};


CacheImportInstalledCommand::CacheImportInstalledCommand(ModManGuiApplication  &app, QWidget *parent)
  : QObject(parent), app(app)
{}

void CacheImportInstalledCommand::execute()
{
    app.refreshMods();

    model = new CacheImportModel(app.cache(), app.modList(), app.modDownloader().modInfoCall(), this);
    connect(model, &CacheImportModel::textOutput, this, &CacheImportInstalledCommand::textOutput);
    CacheImportDialog *dialog = new CacheImportDialog(model, static_cast<QWidget*>(parent()));
    connect(dialog, &QDialog::finished, this, &CacheImportInstalledCommand::dialogFinished);
    dialog->show();
}

void CacheImportInstalledCommand::dialogFinished(int result)
{
    if (result != QDialog::Accepted)
    {
        emit textOutput(QStringLiteral("Import cancelled."));
        emit finished();
        deleteLater();
        return;
    }

    app.refreshMods();

    QList<SteamModInfo> toDownloadMods;
    toCopyMods.clear();
    model->prepareChanges(&toDownloadMods, &toCopyMods);
    model->deleteLater();
    model = nullptr;

    if (toDownloadMods.isEmpty())
        startCopyImports();
    else
    {
        // Start downloads.
        emit textOutput(QStringLiteral("Downloading %1 mods...").arg(toDownloadMods.size()));
        auto *downloader = new GuiModDownloader(app, toDownloadMods, this);
        connect(downloader, &GuiModDownloader::finished, this, &CacheImportInstalledCommand::modDownloadFinished);
        connect(downloader, &GuiModDownloader::textOutput, this, &CacheImportInstalledCommand::textOutput);
        connect(downloader, &GuiModDownloader::beginProgress, this, &CacheImportInstalledCommand::beginProgress);
        connect(downloader, &GuiModDownloader::updateProgress, this, &CacheImportInstalledCommand::updateProgress);

        downloader->execute();
    }
}

void CacheImportInstalledCommand::modDownloadFinished()
{
    emit textOutput("Finished downloading mods. Now checking installed mod versions...");

    app.refreshMods();
    startCopyImports();
}

void CacheImportInstalledCommand::startCopyImports()
{
    for (const auto [installedId, targetId] : toCopyMods)
        copyMod(installedId, targetId);

    emit textOutput("Finished import.");
    app.cache().saveMetadata();
    emit finished();
    deleteLater();
}

bool CacheImportInstalledCommand::copyMod(const QString &installedId, const QString &targetId)
{
    const InstalledMod *im = app.modList().mod(installedId);
    if (!im)
    {
        emit textOutput(QStringLiteral("  Import/check of %1 cancelled: No longer installed.").arg(installedId));
        return false;
    }

    const CachedMod *cm = app.cache().mod(im->id());
    if (cm && cm->latestVersion() && cm->latestVersion()->installed())
    {
        emit textOutput(QStringLiteral("  %1 is up to date.").arg(cm->info().toString()));
        return true;
    }

    QString versionId;
    if (!im->info().isSteam())
        versionId = QStringLiteral("dev");
    else if (!cm)
        versionId = QStringLiteral("000-original");
    else
    {
        int i;
        for (i = 0; i < 100; ++i)
        {
            versionId = QStringLiteral("%1-original").arg(i, 3, 10, QLatin1Char('0'));
            if (!cm->containsVersion(versionId))
                break;
        }
        if (i == 100)
        {
            emit textOutput(QStringLiteral("  Import for %1 cancelled: Exceeded IDs for original versions.").arg(im->info().toString()));
            return false;
        }
    }

    QString errorInfo;
    const CachedVersion *cv = app.cache().addModVersion(targetId, versionId, im->path(), &errorInfo);
    if (cv)
        emit textOutput(QStringLiteral("  Existing version of %1 imported as %2.").arg(cv->info().toString(), versionId));
    else
        emit textOutput(QStringLiteral("  Copying existing version of %1 failed: %2").arg(im->info().toString(), errorInfo));
    return cv;
}

} // namespace iimodmanager

#include "cacheimportinstalledcommand.moc"
