/****************************************************************************
** Meta object code from reading C++ file 'washingmachineconfiguration.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "configurations/washingmachineconfiguration.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'washingmachineconfiguration.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WashingMachineConfiguration_t {
    QByteArrayData data[4];
    char stringdata0[89];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WashingMachineConfiguration_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WashingMachineConfiguration_t qt_meta_stringdata_WashingMachineConfiguration = {
    {
QT_MOC_LITERAL(0, 0, 27), // "WashingMachineConfiguration"
QT_MOC_LITERAL(1, 28, 21), // "washingMachineThingId"
QT_MOC_LITERAL(2, 50, 19), // "optimizationEnabled"
QT_MOC_LITERAL(3, 70, 18) // "maxElectricalPower"

    },
    "WashingMachineConfiguration\0"
    "washingMachineThingId\0optimizationEnabled\0"
    "maxElectricalPower"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WashingMachineConfiguration[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       3,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       4,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, QMetaType::QUuid, 0x00095103,
       2, QMetaType::Bool, 0x00195103,
       3, QMetaType::Double, 0x00195103,

       0        // eod
};

void WashingMachineConfiguration::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{

#ifndef QT_NO_PROPERTIES
    if (_c == QMetaObject::ReadProperty) {
        auto *_t = reinterpret_cast<WashingMachineConfiguration *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QUuid*>(_v) = _t->washingMachineThingId(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->optimizationEnabled(); break;
        case 2: *reinterpret_cast< double*>(_v) = _t->maxElectricalPower(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = reinterpret_cast<WashingMachineConfiguration *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setWashingMachineThingId(*reinterpret_cast< QUuid*>(_v)); break;
        case 1: _t->setOptimizationEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 2: _t->setMaxElectricalPower(*reinterpret_cast< double*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject WashingMachineConfiguration::staticMetaObject = { {
    nullptr,
    qt_meta_stringdata_WashingMachineConfiguration.data,
    qt_meta_data_WashingMachineConfiguration,
    qt_static_metacall,
    nullptr,
    nullptr
} };

QT_WARNING_POP
QT_END_MOC_NAMESPACE
