/****************************************************************************
** Meta object code from reading C++ file 'javascript.hh'
**
** Created: Tue May 6 16:33:09 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "gecode/serialization/javascript.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'javascript.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_Gecode__Serialization__GJSSpace[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      43,   33,   32,   32, 0x0a,
      98,   89,   76,   32, 0x0a,
     133,  129,   76,   32, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Gecode__Serialization__GJSSpace[] = {
    "Gecode::Serialization::GJSSpace\0\0"
    "name,args\0constraint(QString,QScriptValue)\0"
    "QScriptValue\0vti,args\0"
    "variable(QString,QScriptValue)\0a,b\0"
    "pair(QScriptValue,QScriptValue)\0"
};

const QMetaObject Gecode::Serialization::GJSSpace::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Gecode__Serialization__GJSSpace,
      qt_meta_data_Gecode__Serialization__GJSSpace, 0 }
};

const QMetaObject *Gecode::Serialization::GJSSpace::metaObject() const
{
    return &staticMetaObject;
}

void *Gecode::Serialization::GJSSpace::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Gecode__Serialization__GJSSpace))
	return static_cast<void*>(const_cast< GJSSpace*>(this));
    if (!strcmp(_clname, "QScriptable"))
	return static_cast< QScriptable*>(const_cast< GJSSpace*>(this));
    return QObject::qt_metacast(_clname);
}

int Gecode::Serialization::GJSSpace::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: constraint((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QScriptValue(*)>(_a[2]))); break;
        case 1: { QScriptValue _r = variable((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QScriptValue(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QScriptValue*>(_a[0]) = _r; }  break;
        case 2: { QScriptValue _r = pair((*reinterpret_cast< QScriptValue(*)>(_a[1])),(*reinterpret_cast< QScriptValue(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QScriptValue*>(_a[0]) = _r; }  break;
        }
        _id -= 3;
    }
    return _id;
}
