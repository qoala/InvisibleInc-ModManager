#ifndef OPENSPECFILECOMMAND_H
#define OPENSPECFILECOMMAND_H

#include <QObject>
#include <modspec.h>

namespace iimodmanager {

class ModManGuiApplication;


class OpenSpecFileCommand : public QObject
{
    Q_OBJECT

public:
    OpenSpecFileCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void started();
    void finished();
    void textOutput(QString value);
    void modSpecReady(const QList<SpecMod> specMods);

private:
    ModManGuiApplication &app;
    ModSpec *inputSpec;

    void handleFile(const QString &filename, const QByteArray &fileContent);
};

} // namespace iimodmanager

#endif // OPENSPECFILECOMMAND_H
