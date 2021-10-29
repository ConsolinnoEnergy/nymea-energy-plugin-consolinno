QT -= gui
QT += network

TARGET = $$qtLibraryTarget(nymea_experiencepluginconsolinno)
TEMPLATE = lib

CONFIG += plugin link_pkgconfig c++11
PKGCONFIG += nymea

gcc {
    COMPILER_VERSION = $$system($$QMAKE_CXX " -dumpversion")
    COMPILER_MAJOR_VERSION = $$str_member($$COMPILER_VERSION)
    greaterThan(COMPILER_MAJOR_VERSION, 7): QMAKE_CXXFLAGS += -Wno-deprecated-copy
}

HEADERS += experiencepluginconsolinno.h \
    consolinnojsonhandler.h \
    energyengine.h \
    hemsconfiguration.h \
    hemsoptimizerengine.h

SOURCES += experiencepluginconsolinno.cpp \
    consolinnojsonhandler.cpp \
    energyengine.cpp \
    hemsconfiguration.cpp \
    hemsoptimizerengine.cpp

target.path = $$[QT_INSTALL_LIBS]/nymea/experiences/
INSTALLS += target

# Install translation files
TRANSLATIONS *= $$files($${_PRO_FILE_PWD_}/translations/*ts, true)
lupdate.depends = FORCE
lupdate.depends += qmake_all
lupdate.commands = lupdate -recursive -no-obsolete $${_PRO_FILE_PWD_}/experience.pro
QMAKE_EXTRA_TARGETS += lupdate

# make lrelease to build .qm from .ts
lrelease.depends = FORCE
lrelease.commands += lrelease $$files($$_PRO_FILE_PWD_/translations/*.ts, true);
QMAKE_EXTRA_TARGETS += lrelease

translations.depends += lrelease
translations.path = /usr/share/nymea/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm
INSTALLS += translations
