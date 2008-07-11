/****************************************************************************
** Meta object code from reading C++ file 'addvisualisationdialog.hh'
**
** Created: Tue May 6 16:33:09 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "gecode/gist/addvisualisationdialog.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'addvisualisationdialog.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_Gecode__Gist__AddVisualisationDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      38,   37,   37,   37, 0x08,
      84,   37,   37,   37, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Gecode__Gist__AddVisualisationDialog[] = {
    "Gecode::Gist::AddVisualisationDialog\0"
    "\0on_variablesListWidget_itemSelectionChanged()\0"
    "on_visualisationsListWidget_itemSelectionChanged()\0"
};

const QMetaObject Gecode::Gist::AddVisualisationDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Gecode__Gist__AddVisualisationDialog,
      qt_meta_data_Gecode__Gist__AddVisualisationDialog, 0 }
};

const QMetaObject *Gecode::Gist::AddVisualisationDialog::metaObject() const
{
    return &staticMetaObject;
}

void *Gecode::Gist::AddVisualisationDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Gecode__Gist__AddVisualisationDialog))
	return static_cast<void*>(const_cast< AddVisualisationDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int Gecode::Gist::AddVisualisationDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_variablesListWidget_itemSelectionChanged(); break;
        case 1: on_visualisationsListWidget_itemSelectionChanged(); break;
        }
        _id -= 2;
    }
    return _id;
}
