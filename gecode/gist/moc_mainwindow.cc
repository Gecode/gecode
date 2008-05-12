/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.hh'
**
** Created: Mon May 12 11:35:02 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "gecode/gist/mainwindow.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Gecode__Gist__GistMainWindow[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      45,   30,   29,   29, 0x09,
      76,   29,   29,   29, 0x09,
      90,   84,   29,   29, 0x09,
     108,   29,   29,   29, 0x29,

       0        // eod
};

static const char qt_meta_stringdata_Gecode__Gist__GistMainWindow[] = {
    "Gecode::Gist::GistMainWindow\0\0"
    "stats,finished\0statusChanged(Statistics,bool)\0"
    "about()\0setup\0preferences(bool)\0"
    "preferences()\0"
};

const QMetaObject Gecode::Gist::GistMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_Gecode__Gist__GistMainWindow,
      qt_meta_data_Gecode__Gist__GistMainWindow, 0 }
};

const QMetaObject *Gecode::Gist::GistMainWindow::metaObject() const
{
    return &staticMetaObject;
}

void *Gecode::Gist::GistMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Gecode__Gist__GistMainWindow))
	return static_cast<void*>(const_cast< GistMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int Gecode::Gist::GistMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: statusChanged((*reinterpret_cast< const Statistics(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: about(); break;
        case 2: preferences((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: preferences(); break;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
