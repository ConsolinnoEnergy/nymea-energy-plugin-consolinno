/****************************************************************************
** Meta object code from reading C++ file 'heatingrodconfiguration.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "configurations/heatingrodconfiguration.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'heatingrodconfiguration.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_HeatingRodConfiguration_t {
    QByteArrayData data[5];
    char stringdata0[105];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_HeatingRodConfiguration_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_HeatingRodConfiguration_t qt_meta_stringdata_HeatingRodConfiguration = {
    {
QT_MOC_LITERAL(0, 0, 23), // "HeatingRodConfiguration"
QT_MOC_LITERAL(1, 24, 17), // "heatingRodThingId"
QT_MOC_LITERAL(2, 42, 19), // "optimizationEnabled"
QT_MOC_LITERAL(3, 62, 18), // "maxElectricalPower"
QT_MOC_LITERAL(4, 81, 23) // "controllableLocalSystem"

    },
    "HeatingRodConfiguration\0heatingRodThingId\0"
    "optimizationEnabled\0maxElectricalPower\0"
    "controllableLocalSystem"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HeatingRodConfiguration[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       4,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       4,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, QMetaType::QUuid, 0x00095103,
       2, QMetaType::Bool, 0x00195103,
       3, QMetaType::Double, 0x00195103,
       4, QMetaType::Bool, 0x00195103,

       0        // eod
};

void HeatingRodConfiguration::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{

#ifndef QT_NO_PROPERTIES
    if (_c == QMetaObject::ReadProperty) {
        auto *_t = reinterpret_cast<HeatingRodConfiguration *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QUuid*>(_v) = _t->heatingRodThingId(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->optimizationEnabled(); break;
        case 2: *reinterpret_cast< double*>(_v) = _t->maxElectricalPower(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->controllableLocalSystem(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = reinterpret_cast<HeatingRodConfiguration *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setHeatingRodThingId(*reinterpret_cast< QUuid*>(_v)); break;
        case 1: _t->setOptimizationEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 2: _t->setMaxElectricalPower(*reinterpret_cast< double*>(_v)); break;
        case 3: _t->setControllableLocalSystem(*reinterpret_cast< bool*>(_v)); break;
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

QT_INIT_METAOBJECT const QMetaObject HeatingRodConfiguration::staticMetaObject = { {
    nullptr,
    qt_meta_stringdata_HeatingRodConfiguration.data,
    qt_meta_data_HeatingRodConfiguration,
    qt_static_metacall,
    nullptr,
    nullptr
} };

QT_WARNING_POP
QT_END_MOC_NAMESPACE
