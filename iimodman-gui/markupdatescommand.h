#ifndef MARKUPDATESCOMMAND_H
#define MARKUPDATESCOMMAND_H

# include<QObject>

namespace iimodmanager {

class ModManGuiApplication;
class ModSpecPreviewModel;


class MarkUpdatesCommand : public QObject
{
    Q_OBJECT

public:
    MarkUpdatesCommand(ModManGuiApplication &app, ModSpecPreviewModel *preview, QObject *parent = nullptr);

    void execute();

signals:
    void finished();

private:
    ModManGuiApplication &app;
    ModSpecPreviewModel *preview;
};

} // namespace iimodmanager

#endif // MARKUPDATESCOMMAND_H



