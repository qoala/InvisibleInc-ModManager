#ifndef INSTALLEDSYNCFILECOMMAND_H
#define INSTALLEDSYNCFILECOMMAND_H

# include<QObject>
# include<modspec.h>

namespace iimodmanager {

class ModManGuiApplication;


class InstalledSyncFileCommand : public QObject
{
    Q_OBJECT

public:
    InstalledSyncFileCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void textOutput(QString value);
    void finished();

private:
    void handleFile(const QString &filename, const QByteArray &fileContent);

    ModManGuiApplication &app;

    ModSpec inputSpec;
};

} // namespace iimodmanager

#endif // INSTALLEDSYNCFILECOMMAND_H


