#include "cacheimportinstalledcommand.h"
#include "cacheimportmodel.h"
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
    QList<std::pair<QString, QString>> toCopyMods;
    model->prepareChanges(&toDownloadMods, &toCopyMods);

    emit finished();
    deleteLater();
    return;
}

} // namespace iimodmanager

#include "cacheimportinstalledcommand.moc"
