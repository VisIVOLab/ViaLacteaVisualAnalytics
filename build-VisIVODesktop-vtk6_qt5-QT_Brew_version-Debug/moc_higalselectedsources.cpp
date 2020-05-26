/****************************************************************************
** Meta object code from reading C++ file 'higalselectedsources.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Code/src/higalselectedsources.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'higalselectedsources.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_HigalSelectedSources_t {
    QByteArrayData data[25];
    char stringdata0[371];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_HigalSelectedSources_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_HigalSelectedSources_t qt_meta_stringdata_HigalSelectedSources = {
    {
QT_MOC_LITERAL(0, 0, 20), // "HigalSelectedSources"
QT_MOC_LITERAL(1, 21, 13), // "plotNewWindow"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 24), // "on_datasetButton_clicked"
QT_MOC_LITERAL(4, 61, 26), // "on_selectAllButton_clicked"
QT_MOC_LITERAL(5, 88, 28), // "on_deselectAllButton_clicked"
QT_MOC_LITERAL(6, 117, 24), // "updateExistingWindowMenu"
QT_MOC_LITERAL(7, 142, 20), // "on_sedButton_clicked"
QT_MOC_LITERAL(8, 163, 18), // "sourceChangedEvent"
QT_MOC_LITERAL(9, 182, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(10, 199, 3), // "cur"
QT_MOC_LITERAL(11, 203, 3), // "pre"
QT_MOC_LITERAL(12, 207, 20), // "itemSelectionChanged"
QT_MOC_LITERAL(13, 228, 11), // "itemPressed"
QT_MOC_LITERAL(14, 240, 17), // "drawSingleEllipse"
QT_MOC_LITERAL(15, 258, 11), // "vtkEllipse*"
QT_MOC_LITERAL(16, 270, 7), // "ellipse"
QT_MOC_LITERAL(17, 278, 10), // "closeEvent"
QT_MOC_LITERAL(18, 289, 12), // "QCloseEvent*"
QT_MOC_LITERAL(19, 302, 5), // "event"
QT_MOC_LITERAL(20, 308, 27), // "on_tabWidget_currentChanged"
QT_MOC_LITERAL(21, 336, 5), // "index"
QT_MOC_LITERAL(22, 342, 10), // "setConnect"
QT_MOC_LITERAL(23, 353, 12), // "QListWidget*"
QT_MOC_LITERAL(24, 366, 4) // "list"

    },
    "HigalSelectedSources\0plotNewWindow\0\0"
    "on_datasetButton_clicked\0"
    "on_selectAllButton_clicked\0"
    "on_deselectAllButton_clicked\0"
    "updateExistingWindowMenu\0on_sedButton_clicked\0"
    "sourceChangedEvent\0QListWidgetItem*\0"
    "cur\0pre\0itemSelectionChanged\0itemPressed\0"
    "drawSingleEllipse\0vtkEllipse*\0ellipse\0"
    "closeEvent\0QCloseEvent*\0event\0"
    "on_tabWidget_currentChanged\0index\0"
    "setConnect\0QListWidget*\0list"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HigalSelectedSources[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    0,   80,    2, 0x08 /* Private */,
       4,    0,   81,    2, 0x08 /* Private */,
       5,    0,   82,    2, 0x08 /* Private */,
       6,    0,   83,    2, 0x08 /* Private */,
       7,    0,   84,    2, 0x08 /* Private */,
       8,    2,   85,    2, 0x08 /* Private */,
      12,    0,   90,    2, 0x08 /* Private */,
      13,    1,   91,    2, 0x08 /* Private */,
      14,    1,   94,    2, 0x08 /* Private */,
      17,    1,   97,    2, 0x08 /* Private */,
      20,    1,  100,    2, 0x08 /* Private */,
      22,    1,  103,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9, 0x80000000 | 9,   10,   11,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void, 0x80000000 | 23,   24,

       0        // eod
};

void HigalSelectedSources::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HigalSelectedSources *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->plotNewWindow(); break;
        case 1: _t->on_datasetButton_clicked(); break;
        case 2: _t->on_selectAllButton_clicked(); break;
        case 3: _t->on_deselectAllButton_clicked(); break;
        case 4: _t->updateExistingWindowMenu(); break;
        case 5: _t->on_sedButton_clicked(); break;
        case 6: _t->sourceChangedEvent((*reinterpret_cast< QListWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QListWidgetItem*(*)>(_a[2]))); break;
        case 7: _t->itemSelectionChanged(); break;
        case 8: _t->itemPressed((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 9: _t->drawSingleEllipse((*reinterpret_cast< vtkEllipse*(*)>(_a[1]))); break;
        case 10: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 11: _t->on_tabWidget_currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->setConnect((*reinterpret_cast< QListWidget*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QListWidget* >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject HigalSelectedSources::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_HigalSelectedSources.data,
    qt_meta_data_HigalSelectedSources,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *HigalSelectedSources::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HigalSelectedSources::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_HigalSelectedSources.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int HigalSelectedSources::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
