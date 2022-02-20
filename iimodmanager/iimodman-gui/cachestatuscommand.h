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
    void textOutput(QString value);
    void finished();

private:
    ModManGuiApplication &app;
};

} // namespace iimodmanager

#endif // CACHESTATUSCOMMAND_H

