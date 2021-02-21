TEMPLATE = subdirs

SUBDIRS += \
    iimodman-cli \
    iimodman-lib

iimodman-cli.depends = iimodman-lib
