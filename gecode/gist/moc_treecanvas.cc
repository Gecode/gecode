/****************************************************************************
** Meta object code from reading C++ file 'treecanvas.hh'
**
** Created: Sun May 11 18:27:15 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "gecode/gist/treecanvas.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'treecanvas.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Gecode__Gist__SearcherThread[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      34,   30,   29,   29, 0x05,
      50,   29,   29,   29, 0x05,
      70,   29,   29,   29, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Gecode__Gist__SearcherThread[] = {
    "Gecode::Gist::SearcherThread\0\0w,h\0"
    "update(int,int)\0statusChanged(bool)\0"
    "scaleChanged(int)\0"
};

const QMetaObject Gecode::Gist::SearcherThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_Gecode__Gist__SearcherThread,
      qt_meta_data_Gecode__Gist__SearcherThread, 0 }
};

const QMetaObject *Gecode::Gist::SearcherThread::metaObject() const
{
    return &staticMetaObject;
}

void *Gecode::Gist::SearcherThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Gecode__Gist__SearcherThread))
	return static_cast<void*>(const_cast< SearcherThread*>(this));
    return QThread::qt_metacast(_clname);
}

int Gecode::Gist::SearcherThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: update((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: statusChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: scaleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void Gecode::Gist::SearcherThread::update(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Gecode::Gist::SearcherThread::statusChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Gecode::Gist::SearcherThread::scaleChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_Gecode__Gist__TreeCanvasImpl[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      49,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      30,   29,   29,   29, 0x05,
      48,   29,   29,   29, 0x05,
      70,   29,   29,   29, 0x05,
     104,  102,   29,   29, 0x05,
     135,  102,   29,   29, 0x05,
     202,  195,   29,   29, 0x05,
     247,  243,   29,   29, 0x05,

 // slots: signature, parameters, type, tag, flags
     278,  271,   29,   29, 0x0a,
     293,   29,   29,   29, 0x0a,
     305,   29,   29,   29, 0x0a,
     317,   29,   29,   29, 0x0a,
     332,   29,   29,   29, 0x0a,
     345,   29,   29,   29, 0x0a,
     357,   29,   29,   29, 0x0a,
     376,   29,   29,   29, 0x0a,
     384,   29,   29,   29, 0x0a,
     396,   29,   29,   29, 0x0a,
     416,   29,   29,   29, 0x0a,
     437,   29,   29,   29, 0x0a,
     454,   29,   29,   29, 0x0a,
     468,   29,   29,   29, 0x0a,
     481,   29,   29,   29, 0x0a,
     489,   29,   29,   29, 0x0a,
     497,   29,   29,   29, 0x0a,
     507,   29,   29,   29, 0x0a,
     517,   29,   29,   29, 0x0a,
     528,   29,   29,   29, 0x0a,
     543,  538,   29,   29, 0x0a,
     560,   29,   29,   29, 0x2a,
     573,   29,   29,   29, 0x0a,
     586,  243,   29,   29, 0x0a,
     607,   29,   29,   29, 0x0a,
     617,   29,   29,   29, 0x0a,
     633,  631,   29,   29, 0x0a,
     657,  631,   29,   29, 0x0a,
     680,   29,  675,   29, 0x0a,
     700,   29,  675,   29, 0x0a,
     716,  714,   29,   29, 0x0a,
     732,   29,  675,   29, 0x0a,
     757,  631,   29,   29, 0x0a,
     786,   29,   29,   29, 0x0a,
     805,  802,   29,   29, 0x0a,
     870,  846,   29,   29, 0x0a,
     929,  916,   29,   29, 0x2a,
     967,   29,   29,   29, 0x0a,
     986,   29,   29,   29, 0x0a,
     995,   29,   29,   29, 0x0a,
    1008, 1004,   29,   29, 0x0a,
    1028,   29,   29,   29, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Gecode__Gist__TreeCanvasImpl[] = {
    "Gecode::Gist::TreeCanvasImpl\0\0"
    "scaleChanged(int)\0autoZoomChanged(bool)\0"
    "contextMenu(QContextMenuEvent*)\0,\0"
    "statusChanged(Statistics,bool)\0"
    "currentNodeChanged(Gecode::Space*,Gecode::Gist::NodeStatus)\0"
    "vm,pit\0inspect(Gecode::Reflection::VarMap&,int)\0"
    "pit\0pointInTimeChanged(int)\0scale0\0"
    "scaleTree(int)\0searchAll()\0searchOne()\0"
    "toggleHidden()\0hideFailed()\0unhideAll()\0"
    "exportPostscript()\0print()\0zoomToFit()\0"
    "centerCurrentNode()\0inspectCurrentNode()\0"
    "toggleHeatView()\0analyzeTree()\0"
    "stopSearch()\0reset()\0navUp()\0navDown()\0"
    "navLeft()\0navRight()\0navRoot()\0back\0"
    "navNextSol(bool)\0navNextSol()\0"
    "navPrevSol()\0markCurrentNode(int)\0"
    "setPath()\0inspectPath()\0b\0"
    "setAutoHideFailed(bool)\0setAutoZoom(bool)\0"
    "bool\0getAutoHideFailed()\0getAutoZoom()\0"
    "i\0setRefresh(int)\0getSmoothScrollAndZoom()\0"
    "setSmoothScrollAndZoom(bool)\0"
    "resizeToOuter()\0vm\0"
    "getRootVars(Gecode::Reflection::VarMap&)\0"
    "vars,visType,windowName\0"
    "addVisualisation(QStringList,QString,QString)\0"
    "vars,visType\0addVisualisation(QStringList,QString)\0"
    "addVisualisation()\0finish()\0update()\0"
    "w,h\0layoutDone(int,int)\0statusChanged(bool)\0"
};

const QMetaObject Gecode::Gist::TreeCanvasImpl::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Gecode__Gist__TreeCanvasImpl,
      qt_meta_data_Gecode__Gist__TreeCanvasImpl, 0 }
};

const QMetaObject *Gecode::Gist::TreeCanvasImpl::metaObject() const
{
    return &staticMetaObject;
}

void *Gecode::Gist::TreeCanvasImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Gecode__Gist__TreeCanvasImpl))
	return static_cast<void*>(const_cast< TreeCanvasImpl*>(this));
    return QWidget::qt_metacast(_clname);
}

int Gecode::Gist::TreeCanvasImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: scaleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: autoZoomChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: contextMenu((*reinterpret_cast< QContextMenuEvent*(*)>(_a[1]))); break;
        case 3: statusChanged((*reinterpret_cast< const Statistics(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: currentNodeChanged((*reinterpret_cast< Gecode::Space*(*)>(_a[1])),(*reinterpret_cast< Gecode::Gist::NodeStatus(*)>(_a[2]))); break;
        case 5: inspect((*reinterpret_cast< Gecode::Reflection::VarMap(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: pointInTimeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: scaleTree((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: searchAll(); break;
        case 9: searchOne(); break;
        case 10: toggleHidden(); break;
        case 11: hideFailed(); break;
        case 12: unhideAll(); break;
        case 13: exportPostscript(); break;
        case 14: print(); break;
        case 15: zoomToFit(); break;
        case 16: centerCurrentNode(); break;
        case 17: inspectCurrentNode(); break;
        case 18: toggleHeatView(); break;
        case 19: analyzeTree(); break;
        case 20: stopSearch(); break;
        case 21: reset(); break;
        case 22: navUp(); break;
        case 23: navDown(); break;
        case 24: navLeft(); break;
        case 25: navRight(); break;
        case 26: navRoot(); break;
        case 27: navNextSol((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 28: navNextSol(); break;
        case 29: navPrevSol(); break;
        case 30: markCurrentNode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 31: setPath(); break;
        case 32: inspectPath(); break;
        case 33: setAutoHideFailed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 34: setAutoZoom((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 35: { bool _r = getAutoHideFailed();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 36: { bool _r = getAutoZoom();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 37: setRefresh((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 38: { bool _r = getSmoothScrollAndZoom();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 39: setSmoothScrollAndZoom((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 40: resizeToOuter(); break;
        case 41: getRootVars((*reinterpret_cast< Gecode::Reflection::VarMap(*)>(_a[1]))); break;
        case 42: addVisualisation((*reinterpret_cast< QStringList(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 43: addVisualisation((*reinterpret_cast< QStringList(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 44: addVisualisation(); break;
        case 45: finish(); break;
        case 46: update(); break;
        case 47: layoutDone((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 48: statusChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        }
        _id -= 49;
    }
    return _id;
}

// SIGNAL 0
void Gecode::Gist::TreeCanvasImpl::scaleChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Gecode::Gist::TreeCanvasImpl::autoZoomChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Gecode::Gist::TreeCanvasImpl::contextMenu(QContextMenuEvent * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Gecode::Gist::TreeCanvasImpl::statusChanged(const Statistics & _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Gecode::Gist::TreeCanvasImpl::currentNodeChanged(Gecode::Space * _t1, Gecode::Gist::NodeStatus _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Gecode::Gist::TreeCanvasImpl::inspect(Gecode::Reflection::VarMap & _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Gecode::Gist::TreeCanvasImpl::pointInTimeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
static const uint qt_meta_data_Gecode__Gist__TreeCanvas[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      28,   26,   25,   25, 0x05,

 // slots: signature, parameters, type, tag, flags
      59,   25,   25,   25, 0x08,
     101,   26,   25,   25, 0x08,
     171,   26,   25,   25, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Gecode__Gist__TreeCanvas[] = {
    "Gecode::Gist::TreeCanvas\0\0,\0"
    "statusChanged(Statistics,bool)\0"
    "on_canvas_contextMenu(QContextMenuEvent*)\0"
    "on_canvas_currentNodeChanged(Gecode::Space*,Gecode::Gist::NodeStatus)\0"
    "on_canvas_statusChanged(Statistics,bool)\0"
};

const QMetaObject Gecode::Gist::TreeCanvas::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Gecode__Gist__TreeCanvas,
      qt_meta_data_Gecode__Gist__TreeCanvas, 0 }
};

const QMetaObject *Gecode::Gist::TreeCanvas::metaObject() const
{
    return &staticMetaObject;
}

void *Gecode::Gist::TreeCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Gecode__Gist__TreeCanvas))
	return static_cast<void*>(const_cast< TreeCanvas*>(this));
    return QWidget::qt_metacast(_clname);
}

int Gecode::Gist::TreeCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: statusChanged((*reinterpret_cast< const Statistics(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: on_canvas_contextMenu((*reinterpret_cast< QContextMenuEvent*(*)>(_a[1]))); break;
        case 2: on_canvas_currentNodeChanged((*reinterpret_cast< Gecode::Space*(*)>(_a[1])),(*reinterpret_cast< Gecode::Gist::NodeStatus(*)>(_a[2]))); break;
        case 3: on_canvas_statusChanged((*reinterpret_cast< const Statistics(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Gecode::Gist::TreeCanvas::statusChanged(const Statistics & _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
