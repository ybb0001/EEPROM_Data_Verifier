/****************************************************************************
** Meta object code from reading C++ file 'EEPROM_Data_Verifier.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../EEPROM_Data_Verifier.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EEPROM_Data_Verifier.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_EEPROM_Data_Verifier_t {
    QByteArrayData data[62];
    char stringdata0[1102];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_EEPROM_Data_Verifier_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_EEPROM_Data_Verifier_t qt_meta_stringdata_EEPROM_Data_Verifier = {
    {
QT_MOC_LITERAL(0, 0, 20), // "EEPROM_Data_Verifier"
QT_MOC_LITERAL(1, 21, 28), // "on_pushButton_parser_clicked"
QT_MOC_LITERAL(2, 50, 0), // ""
QT_MOC_LITERAL(3, 51, 26), // "on_pushButton_dump_clicked"
QT_MOC_LITERAL(4, 78, 28), // "on_pushButton_folder_clicked"
QT_MOC_LITERAL(5, 107, 36), // "on_pushButton_folder_sorting_..."
QT_MOC_LITERAL(6, 144, 27), // "on_pushButton_clear_clicked"
QT_MOC_LITERAL(7, 172, 32), // "on_pushButton_dump_value_clicked"
QT_MOC_LITERAL(8, 205, 30), // "on_pushButton_dump_SFR_clicked"
QT_MOC_LITERAL(9, 236, 30), // "on_pushButton_dump_LSC_clicked"
QT_MOC_LITERAL(10, 267, 30), // "on_pushButton_dump_MTK_clicked"
QT_MOC_LITERAL(11, 298, 32), // "on_pushButton_Dump_Drift_clicked"
QT_MOC_LITERAL(12, 331, 11), // "selectModel"
QT_MOC_LITERAL(13, 343, 13), // "DisplayOutput"
QT_MOC_LITERAL(14, 357, 16), // "parameterDisplay"
QT_MOC_LITERAL(15, 374, 19), // "load_EEPROM_Address"
QT_MOC_LITERAL(16, 394, 19), // "save_EEPROM_Address"
QT_MOC_LITERAL(17, 414, 10), // "load_Panel"
QT_MOC_LITERAL(18, 425, 10), // "dump_Check"
QT_MOC_LITERAL(19, 436, 13), // "get_Data_Type"
QT_MOC_LITERAL(20, 450, 1), // "x"
QT_MOC_LITERAL(21, 452, 18), // "get_Data_TypeTchar"
QT_MOC_LITERAL(22, 471, 10), // "TCHAR[256]"
QT_MOC_LITERAL(23, 482, 3), // "Str"
QT_MOC_LITERAL(24, 486, 14), // "CheckSum_Check"
QT_MOC_LITERAL(25, 501, 15), // "info_Data_Parse"
QT_MOC_LITERAL(26, 517, 14), // "Oppo_AWB_Parse"
QT_MOC_LITERAL(27, 532, 5), // "group"
QT_MOC_LITERAL(28, 538, 14), // "VIVO_AWB_Parse"
QT_MOC_LITERAL(29, 553, 14), // "SONY_AWB_Parse"
QT_MOC_LITERAL(30, 568, 16), // "XiaoMi_AWB_Parse"
QT_MOC_LITERAL(31, 585, 14), // "MOTO_AWB_Parse"
QT_MOC_LITERAL(32, 600, 15), // "HONOR_AWB_Parse"
QT_MOC_LITERAL(33, 616, 19), // "History_Date_Parser"
QT_MOC_LITERAL(34, 636, 9), // "LSC_Parse"
QT_MOC_LITERAL(35, 646, 5), // "start"
QT_MOC_LITERAL(36, 652, 13), // "MTK_LSC_Parse"
QT_MOC_LITERAL(37, 666, 19), // "HONOR_MTK_AWB_Parse"
QT_MOC_LITERAL(38, 686, 18), // "vivo_MTK_AWB_Parse"
QT_MOC_LITERAL(39, 705, 13), // "MTK_AWB_Parse"
QT_MOC_LITERAL(40, 719, 13), // "LSI_AWB_Parse"
QT_MOC_LITERAL(41, 733, 11), // "drift_Parse"
QT_MOC_LITERAL(42, 745, 11), // "cross_Parse"
QT_MOC_LITERAL(43, 757, 8), // "af_Parse"
QT_MOC_LITERAL(44, 766, 10), // "PDAF_Parse"
QT_MOC_LITERAL(45, 777, 9), // "QSC_Parse"
QT_MOC_LITERAL(46, 787, 9), // "OIS_Parse"
QT_MOC_LITERAL(47, 797, 9), // "AEC_Parse"
QT_MOC_LITERAL(48, 807, 16), // "XiaoMi_Seg_Check"
QT_MOC_LITERAL(49, 824, 14), // "bin_Area_Check"
QT_MOC_LITERAL(50, 839, 15), // "duplicate_Check"
QT_MOC_LITERAL(51, 855, 21), // "value_duplicate_Check"
QT_MOC_LITERAL(52, 877, 16), // "value_Data_Parse"
QT_MOC_LITERAL(53, 894, 14), // "fix_Data_Check"
QT_MOC_LITERAL(54, 909, 13), // "read_Spec_Bin"
QT_MOC_LITERAL(55, 923, 14), // "Reserved_Check"
QT_MOC_LITERAL(56, 938, 29), // "on_pushButton_openBIN_clicked"
QT_MOC_LITERAL(57, 968, 29), // "on_pushButton_saveBIN_clicked"
QT_MOC_LITERAL(58, 998, 30), // "on_pushButton_checkSum_clicked"
QT_MOC_LITERAL(59, 1029, 29), // "on_pushButton_setsave_clicked"
QT_MOC_LITERAL(60, 1059, 30), // "on_pushButton_load_lsc_clicked"
QT_MOC_LITERAL(61, 1090, 11) // "display_EEP"

    },
    "EEPROM_Data_Verifier\0on_pushButton_parser_clicked\0"
    "\0on_pushButton_dump_clicked\0"
    "on_pushButton_folder_clicked\0"
    "on_pushButton_folder_sorting_clicked\0"
    "on_pushButton_clear_clicked\0"
    "on_pushButton_dump_value_clicked\0"
    "on_pushButton_dump_SFR_clicked\0"
    "on_pushButton_dump_LSC_clicked\0"
    "on_pushButton_dump_MTK_clicked\0"
    "on_pushButton_Dump_Drift_clicked\0"
    "selectModel\0DisplayOutput\0parameterDisplay\0"
    "load_EEPROM_Address\0save_EEPROM_Address\0"
    "load_Panel\0dump_Check\0get_Data_Type\0"
    "x\0get_Data_TypeTchar\0TCHAR[256]\0Str\0"
    "CheckSum_Check\0info_Data_Parse\0"
    "Oppo_AWB_Parse\0group\0VIVO_AWB_Parse\0"
    "SONY_AWB_Parse\0XiaoMi_AWB_Parse\0"
    "MOTO_AWB_Parse\0HONOR_AWB_Parse\0"
    "History_Date_Parser\0LSC_Parse\0start\0"
    "MTK_LSC_Parse\0HONOR_MTK_AWB_Parse\0"
    "vivo_MTK_AWB_Parse\0MTK_AWB_Parse\0"
    "LSI_AWB_Parse\0drift_Parse\0cross_Parse\0"
    "af_Parse\0PDAF_Parse\0QSC_Parse\0OIS_Parse\0"
    "AEC_Parse\0XiaoMi_Seg_Check\0bin_Area_Check\0"
    "duplicate_Check\0value_duplicate_Check\0"
    "value_Data_Parse\0fix_Data_Check\0"
    "read_Spec_Bin\0Reserved_Check\0"
    "on_pushButton_openBIN_clicked\0"
    "on_pushButton_saveBIN_clicked\0"
    "on_pushButton_checkSum_clicked\0"
    "on_pushButton_setsave_clicked\0"
    "on_pushButton_load_lsc_clicked\0"
    "display_EEP"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EEPROM_Data_Verifier[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      55,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  289,    2, 0x08 /* Private */,
       3,    0,  290,    2, 0x08 /* Private */,
       4,    0,  291,    2, 0x08 /* Private */,
       5,    0,  292,    2, 0x08 /* Private */,
       6,    0,  293,    2, 0x08 /* Private */,
       7,    0,  294,    2, 0x08 /* Private */,
       8,    0,  295,    2, 0x08 /* Private */,
       9,    0,  296,    2, 0x08 /* Private */,
      10,    0,  297,    2, 0x08 /* Private */,
      11,    0,  298,    2, 0x08 /* Private */,
      12,    0,  299,    2, 0x08 /* Private */,
      13,    0,  300,    2, 0x08 /* Private */,
      14,    0,  301,    2, 0x08 /* Private */,
      15,    0,  302,    2, 0x08 /* Private */,
      16,    0,  303,    2, 0x08 /* Private */,
      17,    0,  304,    2, 0x08 /* Private */,
      18,    0,  305,    2, 0x08 /* Private */,
      19,    1,  306,    2, 0x08 /* Private */,
      21,    1,  309,    2, 0x08 /* Private */,
      24,    0,  312,    2, 0x08 /* Private */,
      25,    0,  313,    2, 0x08 /* Private */,
      26,    1,  314,    2, 0x08 /* Private */,
      28,    1,  317,    2, 0x08 /* Private */,
      29,    1,  320,    2, 0x08 /* Private */,
      30,    1,  323,    2, 0x08 /* Private */,
      31,    1,  326,    2, 0x08 /* Private */,
      32,    1,  329,    2, 0x08 /* Private */,
      33,    0,  332,    2, 0x08 /* Private */,
      34,    2,  333,    2, 0x08 /* Private */,
      36,    1,  338,    2, 0x08 /* Private */,
      37,    1,  341,    2, 0x08 /* Private */,
      38,    1,  344,    2, 0x08 /* Private */,
      39,    1,  347,    2, 0x08 /* Private */,
      40,    1,  350,    2, 0x08 /* Private */,
      41,    0,  353,    2, 0x08 /* Private */,
      42,    0,  354,    2, 0x08 /* Private */,
      43,    0,  355,    2, 0x08 /* Private */,
      44,    0,  356,    2, 0x08 /* Private */,
      45,    0,  357,    2, 0x08 /* Private */,
      46,    0,  358,    2, 0x08 /* Private */,
      47,    0,  359,    2, 0x08 /* Private */,
      48,    0,  360,    2, 0x08 /* Private */,
      49,    0,  361,    2, 0x08 /* Private */,
      50,    0,  362,    2, 0x08 /* Private */,
      51,    0,  363,    2, 0x08 /* Private */,
      52,    0,  364,    2, 0x08 /* Private */,
      53,    0,  365,    2, 0x08 /* Private */,
      54,    0,  366,    2, 0x08 /* Private */,
      55,    0,  367,    2, 0x08 /* Private */,
      56,    0,  368,    2, 0x08 /* Private */,
      57,    0,  369,    2, 0x08 /* Private */,
      58,    0,  370,    2, 0x08 /* Private */,
      59,    0,  371,    2, 0x08 /* Private */,
      60,    0,  372,    2, 0x08 /* Private */,
      61,    0,  373,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Int, QMetaType::Int,   20,
    QMetaType::Int, 0x80000000 | 22,   23,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void,
    QMetaType::Int, QMetaType::Int, QMetaType::Int,   35,   27,
    QMetaType::Int, QMetaType::Int,   35,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void EEPROM_Data_Verifier::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        EEPROM_Data_Verifier *_t = static_cast<EEPROM_Data_Verifier *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_pushButton_parser_clicked(); break;
        case 1: _t->on_pushButton_dump_clicked(); break;
        case 2: _t->on_pushButton_folder_clicked(); break;
        case 3: _t->on_pushButton_folder_sorting_clicked(); break;
        case 4: _t->on_pushButton_clear_clicked(); break;
        case 5: _t->on_pushButton_dump_value_clicked(); break;
        case 6: _t->on_pushButton_dump_SFR_clicked(); break;
        case 7: _t->on_pushButton_dump_LSC_clicked(); break;
        case 8: _t->on_pushButton_dump_MTK_clicked(); break;
        case 9: _t->on_pushButton_Dump_Drift_clicked(); break;
        case 10: _t->selectModel(); break;
        case 11: _t->DisplayOutput(); break;
        case 12: _t->parameterDisplay(); break;
        case 13: _t->load_EEPROM_Address(); break;
        case 14: _t->save_EEPROM_Address(); break;
        case 15: _t->load_Panel(); break;
        case 16: _t->dump_Check(); break;
        case 17: { int _r = _t->get_Data_Type((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 18: { int _r = _t->get_Data_TypeTchar((*reinterpret_cast< TCHAR(*)[256]>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 19: { int _r = _t->CheckSum_Check();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 20: { int _r = _t->info_Data_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 21: _t->Oppo_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 22: _t->VIVO_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->SONY_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: _t->XiaoMi_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: _t->MOTO_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 26: _t->HONOR_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 27: _t->History_Date_Parser(); break;
        case 28: { int _r = _t->LSC_Parse((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 29: { int _r = _t->MTK_LSC_Parse((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 30: _t->HONOR_MTK_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 31: _t->vivo_MTK_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 32: _t->MTK_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 33: _t->LSI_AWB_Parse((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 34: { int _r = _t->drift_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 35: { int _r = _t->cross_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 36: { int _r = _t->af_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 37: { int _r = _t->PDAF_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 38: { int _r = _t->QSC_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 39: { int _r = _t->OIS_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 40: { int _r = _t->AEC_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 41: { int _r = _t->XiaoMi_Seg_Check();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 42: { int _r = _t->bin_Area_Check();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 43: { int _r = _t->duplicate_Check();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 44: { int _r = _t->value_duplicate_Check();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 45: { int _r = _t->value_Data_Parse();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 46: { int _r = _t->fix_Data_Check();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 47: { int _r = _t->read_Spec_Bin();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 48: { int _r = _t->Reserved_Check();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 49: _t->on_pushButton_openBIN_clicked(); break;
        case 50: _t->on_pushButton_saveBIN_clicked(); break;
        case 51: _t->on_pushButton_checkSum_clicked(); break;
        case 52: _t->on_pushButton_setsave_clicked(); break;
        case 53: _t->on_pushButton_load_lsc_clicked(); break;
        case 54: _t->display_EEP(); break;
        default: ;
        }
    }
}

const QMetaObject EEPROM_Data_Verifier::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_EEPROM_Data_Verifier.data,
      qt_meta_data_EEPROM_Data_Verifier,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *EEPROM_Data_Verifier::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EEPROM_Data_Verifier::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_EEPROM_Data_Verifier.stringdata0))
        return static_cast<void*>(const_cast< EEPROM_Data_Verifier*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int EEPROM_Data_Verifier::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 55)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 55;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 55)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 55;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
