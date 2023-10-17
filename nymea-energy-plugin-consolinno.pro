QT -= gui
QT += network
QT += dbus

TARGET = $$qtLibraryTarget(nymea_energypluginconsolinno)
TEMPLATE = lib

CONFIG += plugin link_pkgconfig c++11
PKGCONFIG += nymea nymea-energy

gcc {
    COMPILER_VERSION = $$system($$QMAKE_CXX " -dumpversion")
    COMPILER_MAJOR_VERSION = $$str_member($$COMPILER_VERSION)
    greaterThan(COMPILER_MAJOR_VERSION, 7): QMAKE_CXXFLAGS += -Wno-deprecated-copy
}

HEADERS += \
    configurations/batteryconfiguration.h \
    configurations/chargingconfiguration.h \
    configurations/chargingoptimizationconfiguration.h \
    configurations/chargingsessionconfiguration.h \
    configurations/conemsstate.h \
    configurations/heatingconfiguration.h \
    configurations/pvconfiguration.h \
    configurations/userconfiguration.h \
    configurations/heatingrodconfiguration.h \
    consolinnojsonhandler.h \
    energyengine.h \
    energypluginconsolinno.h

SOURCES += \
    configurations/batteryconfiguration.cpp \
    configurations/chargingconfiguration.cpp \
    configurations/chargingoptimizationconfiguration.cpp \
    configurations/chargingsessionconfiguration.cpp \
    configurations/conemsstate.cpp \
    configurations/heatingconfiguration.cpp \
    configurations/pvconfiguration.cpp \
    configurations/userconfiguration.cpp \
    configurations/heatingrodconfiguration.cpp \
    consolinnojsonhandler.cpp \
    energyengine.cpp \
    energypluginconsolinno.cpp

target.path = $$[QT_INSTALL_LIBS]/nymea/energy/
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
