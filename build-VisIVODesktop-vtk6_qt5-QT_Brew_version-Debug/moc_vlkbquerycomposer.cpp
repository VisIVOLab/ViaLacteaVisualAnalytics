/****************************************************************************
** Meta object code from reading C++ file 'vlkbquerycomposer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Code/src/vlkbquerycomposer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'vlkbquerycomposer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VLKBQueryComposer_t {
    QByteArrayData data[19];
    char stringdata0[320];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VLKBQueryComposer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VLKBQueryComposer_t qt_meta_stringdata_VLKBQueryComposer = {
    {
QT_MOC_LITERAL(0, 0, 17), // "VLKBQueryComposer"
QT_MOC_LITERAL(1, 18, 18), // "tableReplyFinished"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 14), // "QNetworkReply*"
QT_MOC_LITERAL(4, 53, 5), // "reply"
QT_MOC_LITERAL(5, 59, 18), // "availReplyFinished"
QT_MOC_LITERAL(6, 78, 24), // "on_connectButton_clicked"
QT_MOC_LITERAL(7, 103, 17), // "checkAvailability"
QT_MOC_LITERAL(8, 121, 40), // "on_tableListComboBox_currentI..."
QT_MOC_LITERAL(9, 162, 5), // "index"
QT_MOC_LITERAL(10, 168, 19), // "on_okButton_clicked"
QT_MOC_LITERAL(11, 188, 27), // "onAuthenticationRequestSlot"
QT_MOC_LITERAL(12, 216, 6), // "aReply"
QT_MOC_LITERAL(13, 223, 15), // "QAuthenticator*"
QT_MOC_LITERAL(14, 239, 14), // "aAuthenticator"
QT_MOC_LITERAL(15, 254, 18), // "queryReplyFinished"
QT_MOC_LITERAL(16, 273, 11), // "redirectUrl"
QT_MOC_LITERAL(17, 285, 19), // "possibleRedirectUrl"
QT_MOC_LITERAL(18, 305, 14) // "oldRedirectUrl"

    },
    "VLKBQueryComposer\0tableReplyFinished\0"
    "\0QNetworkReply*\0reply\0availReplyFinished\0"
    "on_connectButton_clicked\0checkAvailability\0"
    "on_tableListComboBox_currentIndexChanged\0"
    "index\0on_okButton_clicked\0"
    "onAuthenticationRequestSlot\0aReply\0"
    "QAuthenticator*\0aAuthenticator\0"
    "queryReplyFinished\0redirectUrl\0"
    "possibleRedirectUrl\0oldRedirectUrl"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VLKBQueryComposer[] = {

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
       1,    1,   59,    2, 0x0a /* Public */,
       5,    1,   62,    2, 0x0a /* Public */,
       6,    0,   65,    2, 0x08 /* Private */,
       7,    0,   66,    2, 0x08 /* Private */,
       8,    1,   67,    2, 0x08 /* Private */,
      10,    0,   70,    2, 0x08 /* Private */,
      11,    2,   71,    2, 0x08 /* Private */,
      15,    1,   76,    2, 0x08 /* Private */,
      16,    2,   79,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 13,   12,   14,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::QUrl, QMetaType::QUrl, QMetaType::QUrl,   17,   18,

       0        // eod
};

void VLKBQueryComposer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VLKBQueryComposer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->tableReplyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 1: _t->availReplyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 2: _t->on_connectButton_clicked(); break;
        case 3: _t->checkAvailability(); break;
        case 4: _t->on_tableListComboBox_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->on_okButton_clicked(); break;
        case 6: _t->onAuthenticationRequestSlot((*reinterpret_cast< QNetworkReply*(*)>(_a[1])),(*reinterpret_cast< QAuthenticator*(*)>(_a[2]))); break;
        case 7: _t->queryReplyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 8: { QUrl _r = _t->redirectUrl((*reinterpret_cast< const QUrl(*)>(_a[1])),(*reinterpret_cast< const QUrl(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QUrl*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VLKBQueryComposer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_VLKBQueryComposer.data,
    qt_meta_data_VLKBQueryComposer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VLKBQueryComposer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VLKBQueryComposer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VLKBQueryComposer.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int VLKBQueryComposer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
