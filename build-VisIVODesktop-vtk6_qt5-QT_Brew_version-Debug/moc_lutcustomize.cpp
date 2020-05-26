/****************************************************************************
** Meta object code from reading C++ file 'lutcustomize.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Code/src/lutcustomize.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'lutcustomize.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LutCustomize_t {
    QByteArrayData data[17];
    char stringdata0[235];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LutCustomize_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LutCustomize_t qt_meta_stringdata_LutCustomize = {
    {
QT_MOC_LITERAL(0, 0, 12), // "LutCustomize"
QT_MOC_LITERAL(1, 13, 27), // "on_cancelPushButton_clicked"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 10), // "closeEvent"
QT_MOC_LITERAL(4, 53, 12), // "QCloseEvent*"
QT_MOC_LITERAL(5, 66, 5), // "event"
QT_MOC_LITERAL(6, 72, 31), // "on_ShowColorbarCheckBox_clicked"
QT_MOC_LITERAL(7, 104, 7), // "checked"
QT_MOC_LITERAL(8, 112, 13), // "plotHistogram"
QT_MOC_LITERAL(9, 126, 8), // "drawLine"
QT_MOC_LITERAL(10, 135, 4), // "from"
QT_MOC_LITERAL(11, 140, 2), // "to"
QT_MOC_LITERAL(12, 143, 8), // "setRange"
QT_MOC_LITERAL(13, 152, 25), // "on_fromSlider_sliderMoved"
QT_MOC_LITERAL(14, 178, 8), // "position"
QT_MOC_LITERAL(15, 187, 23), // "on_toSlider_sliderMoved"
QT_MOC_LITERAL(16, 211, 23) // "on_okPushButton_clicked"

    },
    "LutCustomize\0on_cancelPushButton_clicked\0"
    "\0closeEvent\0QCloseEvent*\0event\0"
    "on_ShowColorbarCheckBox_clicked\0checked\0"
    "plotHistogram\0drawLine\0from\0to\0setRange\0"
    "on_fromSlider_sliderMoved\0position\0"
    "on_toSlider_sliderMoved\0on_okPushButton_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LutCustomize[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x08 /* Private */,
       3,    1,   60,    2, 0x08 /* Private */,
       6,    1,   63,    2, 0x08 /* Private */,
       8,    0,   66,    2, 0x08 /* Private */,
       9,    2,   67,    2, 0x08 /* Private */,
      12,    0,   72,    2, 0x08 /* Private */,
      13,    1,   73,    2, 0x08 /* Private */,
      15,    1,   76,    2, 0x08 /* Private */,
      16,    0,   79,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   10,   11,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void,

       0        // eod
};

void LutCustomize::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LutCustomize *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_cancelPushButton_clicked(); break;
        case 1: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 2: _t->on_ShowColorbarCheckBox_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->plotHistogram(); break;
        case 4: _t->drawLine((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 5: _t->setRange(); break;
        case 6: _t->on_fromSlider_sliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->on_toSlider_sliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->on_okPushButton_clicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LutCustomize::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_LutCustomize.data,
    qt_meta_data_LutCustomize,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LutCustomize::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LutCustomize::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LutCustomize.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int LutCustomize::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
