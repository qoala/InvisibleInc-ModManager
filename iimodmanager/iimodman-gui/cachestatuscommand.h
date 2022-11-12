#ifndef CACHESTATUSCOMMAND_H
#define CACHESTATUSCOMMAND_H

# include<QObject>

namespace iimodmanager {

class ModManGuiApplication;


class CacheStatusCommand : public QObject
{
    Q_OBJECT

public:
    CacheStatusCommand(ModManGuiApplication &app, QObject *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);

private:
    ModManGuiApplication &app;
};

} // namespace iimodmanager

#endif // CACHESTATUSCOMMAND_H

