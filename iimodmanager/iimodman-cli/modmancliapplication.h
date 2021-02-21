#ifndef MODMANCLIAPPLICATION_H
#define MODMANCLIAPPLICATION_H

#include <QCoreApplication>
#include <modmanconfig.h>

namespace iimodmanager {

class ModManCliApplication
{
public:
    ModManCliApplication(int &argc, char **argv[]);

    inline ModManConfig &config() { return config_; }
    inline const QString applicationName() const { return app_.applicationName(); }

    static int main(int argc, char *argv[]);

private:
    QCoreApplication app_;
    ModManConfig config_;
};

}  // namespace iimodmanager

#endif // MODMANCLIAPPLICATION_H
