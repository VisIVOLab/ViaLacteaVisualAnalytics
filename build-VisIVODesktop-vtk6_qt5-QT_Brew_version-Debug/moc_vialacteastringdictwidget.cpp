/****************************************************************************
** Meta object code from reading C++ file 'vialacteastringdictwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Code/src/vialacteastringdictwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'vialacteastringdictwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VialacteaStringDictWidget_t {
    QByteArrayData data[16];
    char stringdata0[306];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VialacteaStringDictWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VialacteaStringDictWidget_t qt_meta_stringdata_VialacteaStringDictWidget = {
    {
QT_MOC_LITERAL(0, 0, 25), // "VialacteaStringDictWidget"
QT_MOC_LITERAL(1, 26, 18), // "availReplyFinished"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 14), // "QNetworkReply*"
QT_MOC_LITERAL(4, 61, 5), // "reply"
QT_MOC_LITERAL(5, 67, 27), // "executeQueryTapSchemaTables"
QT_MOC_LITERAL(6, 95, 28), // "executeQueryTapSchemaColumns"
QT_MOC_LITERAL(7, 124, 27), // "onAuthenticationRequestSlot"
QT_MOC_LITERAL(8, 152, 6), // "aReply"
QT_MOC_LITERAL(9, 159, 15), // "QAuthenticator*"
QT_MOC_LITERAL(10, 175, 14), // "aAuthenticator"
QT_MOC_LITERAL(11, 190, 33), // "queryReplyFinishedTapSchemaTa..."
QT_MOC_LITERAL(12, 224, 34), // "queryReplyFinishedTapSchemaCo..."
QT_MOC_LITERAL(13, 259, 11), // "redirectUrl"
QT_MOC_LITERAL(14, 271, 19), // "possibleRedirectUrl"
QT_MOC_LITERAL(15, 291, 14) // "oldRedirectUrl"

    },
    "VialacteaStringDictWidget\0availReplyFinished\0"
    "\0QNetworkReply*\0reply\0executeQueryTapSchemaTables\0"
    "executeQueryTapSchemaColumns\0"
    "onAuthenticationRequestSlot\0aReply\0"
    "QAuthenticator*\0aAuthenticator\0"
    "queryReplyFinishedTapSchemaTables\0"
    "queryReplyFinishedTapSchemaColumns\0"
    "redirectUrl\0possibleRedirectUrl\0"
    "oldRedirectUrl"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VialacteaStringDictWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x08 /* Private */,
       5,    0,   52,    2, 0x08 /* Private */,
       6,    0,   53,    2, 0x08 /* Private */,
       7,    2,   54,    2, 0x08 /* Private */,
      11,    1,   59,    2, 0x08 /* Private */,
      12,    1,   62,    2, 0x08 /* Private */,
      13,    2,   65,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 9,    8,   10,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::QUrl, QMetaType::QUrl, QMetaType::QUrl,   14,   15,

       0        // eod
};

void VialacteaStringDictWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VialacteaStringDictWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->availReplyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 1: _t->executeQueryTapSchemaTables(); break;
        case 2: _t->executeQueryTapSchemaColumns(); break;
        case 3: _t->onAuthenticationRequestSlot((*reinterpret_cast< QNetworkReply*(*)>(_a[1])),(*reinterpret_cast< QAuthenticator*(*)>(_a[2]))); break;
        case 4: _t->queryReplyFinishedTapSchemaTables((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 5: _t->queryReplyFinishedTapSchemaColumns((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 6: { QUrl _r = _t->redirectUrl((*reinterpret_cast< const QUrl(*)>(_a[1])),(*reinterpret_cast< const QUrl(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QUrl*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VialacteaStringDictWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_VialacteaStringDictWidget.data,
    qt_meta_data_VialacteaStringDictWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VialacteaStringDictWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VialacteaStringDictWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VialacteaStringDictWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int VialacteaStringDictWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
