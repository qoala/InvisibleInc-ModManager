#include "applypreviewcommand.h"
#include "modmanguiapplication.h"
#include "util.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringBuilder>
#include <QTimer>
#include <QVBoxLayout>
#include <modcache.h>
#include <modinfo.h>
#include <modlist.h>
#include <modspec.h>

namespace iimodmanager {

class DetailedDialog : public QDialog
{
    Q_OBJECT
public:
    DetailedDialog(const QString &labelText, const QString &detailText, QWidget *parent)
        : QDialog(parent)
    {
        QLabel *label = new QLabel(labelText);
        QPlainTextEdit *detail = new QPlainTextEdit(detailText);
        detail->setReadOnly(true);
        detail->setTabStopDistance(240);
        detail->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

        QDialogButtonBox *buttonBox = new QDialogButtonBox;
        QPushButton *yesButton = buttonBox->addButton(QDialogButtonBox::Yes);
        connect(yesButton, &QAbstractButton::clicked, this, &QDialog::accept);
        QPushButton *noButton = buttonBox->addButton(QDialogButtonBox::No);
        connect(noButton, &QAbstractButton::clicked, this, &QDialog::reject);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(label);
        mainLayout->addWidget(detail);
        mainLayout->addWidget(buttonBox);
        setLayout(mainLayout);

        setWindowTitle(tr("II Mod Manager"));
        setSizeGripEnabled(true);
    }
};


static QString containsNewAliases(const ModCache &cache, const QList<SpecMod> &toAddMods)
{
    QStringList aliasChanges;
    for (const auto &sm : toAddMods)
    {
        const CachedMod *cm = cache.mod(sm.id());
        if (cm && cm->defaultAlias() != sm.alias())
            aliasChanges << cm->info().toString() % " : " % (sm.alias().isEmpty() ? "(cleared)" : sm.alias());
    }
    return aliasChanges.join('\n');
}

static void updateDefaultAliases(ModCache &cache, const QList<SpecMod> &toAddMods)
{
    for (const auto &sm : toAddMods)
    {
        const CachedMod *cm = cache.mod(sm.id());
        if (cm && cm->defaultAlias() != sm.alias())
            cache.setDefaultAlias(cm->id(), sm.alias());
    }
    cache.saveMetadata();
}


ApplyPreviewCommand::ApplyPreviewCommand(ModManGuiApplication  &app, ModSpecPreviewModel *preview, QWidget *parent)
  : QObject(parent), app(app), preview(preview)
{}

void ApplyPreviewCommand::execute()
{
    if (!preview)
    {
        emit finished();
        deleteLater();
        return;
    }

    app.refreshMods();

    preview->prepareChanges(&toAddMods, &toUpdateMods, &toRemoveMods);
    // TODO: Warn if about to delete uncached mod versions.
    // TODO: Confirmation dialog with list of changes.

    const QString aliasChanges = containsNewAliases(app.cache(), toAddMods);
    if (aliasChanges.isEmpty())
        QTimer::singleShot(0, this, &ApplyPreviewCommand::applyChanges);
    else
    {
        DetailedDialog *prompt = new DetailedDialog(
                tr("Preparing to install mods with a different alias than their current default. Update the default alias for these mods?"),
                aliasChanges, static_cast<QWidget*>(parent()));
        connect(prompt, &QDialog::finished, this, &ApplyPreviewCommand::dialogFinished);
        if (app.config().openMaximized())
            prompt->showMaximized();
        else
            prompt->show();
    }
}

void ApplyPreviewCommand::finish()
{
    // Cleanup the preview's state if it applied cleanly.
    if (!preview->isEmpty())
        preview->revert();

    emit finished();
    deleteLater();
}

void ApplyPreviewCommand::dialogFinished(int result)
{
    if (result == QDialog::Accepted)
        updateDefaultAliases(app.cache(), toAddMods);

    applyChanges();
}

void ApplyPreviewCommand::applyChanges()
{
    if (doApply())
        emit textOutput("Sync complete");
    else
        emit textOutput("Sync aborted");

    app.refreshMods();

    // Wait for any refresh callbacks to propagate.
    QTimer::singleShot(0, this, &ApplyPreviewCommand::finish);
}

bool ApplyPreviewCommand::doApply()
{
    emit textOutput("Syncing mods...");
    for (const InstalledMod &sm : toRemoveMods)
        if (!removeMod(sm))
            return false;
    for (const SpecMod &sm : toAddMods)
        if (!installMod(sm))
            return false;
    for (const SpecMod &sm : toUpdateMods)
        if (!updateMod(sm))
            return false;
    return true;
}

bool ApplyPreviewCommand::removeMod(const InstalledMod &im)
{
    QString errorInfo;
    if (app.modList().removeMod(im.id(), &errorInfo))
    {
        emit textOutput(QStringLiteral("  Removed %1 \t%2").arg(
                    util::displayInfo(im.info(), im.alias()),
                    util::displayVersion(im.info().version())));
        return true;
    }
    else
    {
        emit textOutput(QStringLiteral("Failed to remove %1: %2").arg(
                    util::displayInfo(im.info(), im.alias()), errorInfo));
        return false;
    }
}

bool ApplyPreviewCommand::installMod(const SpecMod &sm)
{
    QString errorInfo;
    const InstalledMod *im = app.modList().installMod(sm, &errorInfo);
    if (im)
    {
        emit textOutput(QStringLiteral("  Installed %1 \t%2").arg(
                    util::displayInfo(im->info(), im->alias()),
                    util::displayVersion(im->info().version())));
    }
    else
    {
        const CachedMod *cm = app.cache().mod(sm.id());
        emit textOutput(QStringLiteral("Failed to install %1: %2").arg(
                    cm ? util::displayInfo(cm->info(), sm.alias()) : util::displayInfo(sm), errorInfo));
    }
    return im;
}

bool ApplyPreviewCommand::updateMod(const SpecMod &sm)
{
    const InstalledMod *from = app.modList().mod(sm.id());
    QString fromVersion = from ? from->info().version() : QString();

    QString errorInfo;
    const InstalledMod *im = app.modList().installMod(sm, &errorInfo);
    if (im)
    {
        emit textOutput(QStringLiteral("  Installed %1 \t%2 => %3").arg(
                    util::displayInfo(im->info(), im->alias()),
                    util::displayVersion(fromVersion),
                    util::displayVersion(im->info().version())));
    }
    else
    {
        const CachedMod *cm = app.cache().mod(sm.id());
        emit textOutput(QStringLiteral("Failed to install %1: %2").arg(
                    cm ? util::displayInfo(cm->info(), sm.alias()) : util::displayInfo(sm), errorInfo));
    }
    return im;
}

} // namespace iimodmanager

#include "applypreviewcommand.moc"
