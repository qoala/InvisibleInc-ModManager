#ifndef MODMANCLIAPPLICATION_H
#define MODMANCLIAPPLICATION_H

#include <QCoreApplication>
#include <modmanconfig.h>

namespace iimodmanager {

class ModManCliApplication
{
public:
    ModManCliApplication(int argc, char *argv[]);

    static int main(int argc, char *argv[]);

private:
    QCoreApplication app_;
    ModManConfig config_;

    void printConfigValues();
};

}  // namespace iimodmanager

#endif // MODMANCLIAPPLICATION_H
