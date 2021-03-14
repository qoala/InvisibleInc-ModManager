#ifndef MODMANCLIAPPLICATION_H
#define MODMANCLIAPPLICATION_H

#include <QCoreApplication>
#include <modmanconfig.h>

namespace iimodmanager {

class ModManCliApplication: public QCoreApplication
{
public:
    ModManCliApplication(int &argc, char **argv[]);

    inline ModManConfig &config() { return config_; }

    static int main(int argc, char *argv[]);

private:
    ModManConfig config_;
};

}  // namespace iimodmanager

#endif // MODMANCLIAPPLICATION_H
