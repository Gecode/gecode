/****************************************************************************
** Meta object code from reading C++ file 'preferences.hh'
**
** Created: Sun May 11 18:27:15 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "gecode/gist/preferences.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'preferences.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Gecode__Gist__PreferencesDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      33,   32,   32,   32, 0x09,
      45,   32,   32,   32, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_Gecode__Gist__PreferencesDialog[] = {
    "Gecode::Gist::PreferencesDialog\0\0"
    "writeBack()\0defaults()\0"
};

const QMetaObject Gecode::Gist::PreferencesDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Gecode__Gist__PreferencesDialog,
      qt_meta_data_Gecode__Gist__PreferencesDialog, 0 }
};

const QMetaObject *Gecode::Gist::PreferencesDialog::metaObject() const
{
    return &staticMetaObject;
}

void *Gecode::Gist::PreferencesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Gecode__Gist__PreferencesDialog))
	return static_cast<void*>(const_cast< PreferencesDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int Gecode::Gist::PreferencesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: writeBack(); break;
        case 1: defaults(); break;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
