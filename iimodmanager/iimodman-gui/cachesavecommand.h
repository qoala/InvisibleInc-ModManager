#ifndef CACHESAVECOMMAND_H
#define CACHESAVECOMMAND_H

# include<QFlags>
# include<QObject>

namespace iimodmanager {

class ModManGuiApplication;


class CacheSaveCommand : public QObject
{
    Q_OBJECT

public:
    enum SaveOption
    {
        //! All mods in the cache.
        CACHED_MODS = 1,
        //! All installed mods.
        INSTALLED_MODS = 2,

        //! Include current/latest versions.
        WITH_VERSIONS = 0x100,
    };
    Q_DECLARE_FLAGS(SaveOptions, SaveOption)

    CacheSaveCommand(ModManGuiApplication &app, SaveOptions options, QObject *parent = nullptr);

    void execute();

signals:
    void finished();
    void textOutput(QString value);

private:
    ModManGuiApplication &app;
    SaveOptions options;

    QString generateContent() const;
    void writeFile(QString content);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CacheSaveCommand::SaveOptions)

} // namespace iimodmanager

#endif // CACHESAVECOMMAND_H

