#ifndef CACHESAVECOMMAND_H
#define CACHESAVECOMMAND_H

# include<QObject>

namespace iimodmanager {

class ModManGuiApplication;


class CacheSaveCommand : public QObject
{
    Q_OBJECT

public:
    CacheSaveCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void textOutput(QString value);
    void finished();

private:
    QString generateContent();
    void writeFile(QString content);

    ModManGuiApplication &app;
};

} // namespace iimodmanager

#endif // CACHESAVECOMMAND_H

