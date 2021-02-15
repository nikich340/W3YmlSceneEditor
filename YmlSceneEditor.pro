QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += \
    ryml/src \
    ryml/ext/c4core/src \
    ryml/ext/c4core/src/c4/ext/debugbreak \
    ryml/ext/c4core/src/c4/ext/fast_float/include

HEADERS += \
    mainwindow.h \
    qmenu2.h \
    ryml/ext/c4core/src/c4/allocator.hpp \
    ryml/ext/c4core/src/c4/base64.hpp \
    ryml/ext/c4core/src/c4/bitmask.hpp \
    ryml/ext/c4core/src/c4/blob.hpp \
    ryml/ext/c4core/src/c4/c4_pop.hpp \
    ryml/ext/c4core/src/c4/c4_push.hpp \
    ryml/ext/c4core/src/c4/char_traits.hpp \
    ryml/ext/c4core/src/c4/charconv.hpp \
    ryml/ext/c4core/src/c4/common.hpp \
    ryml/ext/c4core/src/c4/compiler.hpp \
    ryml/ext/c4core/src/c4/config.hpp \
    ryml/ext/c4core/src/c4/cpu.hpp \
    ryml/ext/c4core/src/c4/ctor_dtor.hpp \
    ryml/ext/c4core/src/c4/enum.hpp \
    ryml/ext/c4core/src/c4/error.hpp \
    ryml/ext/c4core/src/c4/export.hpp \
    ryml/ext/c4core/src/c4/ext/debugbreak/debugbreak.h \
    ryml/ext/c4core/src/c4/ext/fast_float.hpp \
    ryml/ext/c4core/src/c4/ext/fast_float/include/fast_float/ascii_number.h \
    ryml/ext/c4core/src/c4/ext/fast_float/include/fast_float/decimal_to_binary.h \
    ryml/ext/c4core/src/c4/ext/fast_float/include/fast_float/fast_float.h \
    ryml/ext/c4core/src/c4/ext/fast_float/include/fast_float/fast_table.h \
    ryml/ext/c4core/src/c4/ext/fast_float/include/fast_float/float_common.h \
    ryml/ext/c4core/src/c4/ext/fast_float/include/fast_float/parse_number.h \
    ryml/ext/c4core/src/c4/ext/fast_float/include/fast_float/simple_decimal_conversion.h \
    ryml/ext/c4core/src/c4/ext/rng/rng.hpp \
    ryml/ext/c4core/src/c4/ext/sg14/inplace_function.h \
    ryml/ext/c4core/src/c4/format.hpp \
    ryml/ext/c4core/src/c4/hash.hpp \
    ryml/ext/c4core/src/c4/language.hpp \
    ryml/ext/c4core/src/c4/memory_resource.hpp \
    ryml/ext/c4core/src/c4/memory_util.hpp \
    ryml/ext/c4core/src/c4/platform.hpp \
    ryml/ext/c4core/src/c4/preprocessor.hpp \
    ryml/ext/c4core/src/c4/restrict.hpp \
    ryml/ext/c4core/src/c4/span.hpp \
    ryml/ext/c4core/src/c4/std/std.hpp \
    ryml/ext/c4core/src/c4/std/string.hpp \
    ryml/ext/c4core/src/c4/std/tuple.hpp \
    ryml/ext/c4core/src/c4/std/vector.hpp \
    ryml/ext/c4core/src/c4/substr.hpp \
    ryml/ext/c4core/src/c4/substr_fwd.hpp \
    ryml/ext/c4core/src/c4/szconv.hpp \
    ryml/ext/c4core/src/c4/time.hpp \
    ryml/ext/c4core/src/c4/type_name.hpp \
    ryml/ext/c4core/src/c4/types.hpp \
    ryml/ext/c4core/src/c4/unrestrict.hpp \
    ryml/ext/c4core/src/c4/windows.hpp \
    ryml/ext/c4core/src/c4/windows_pop.hpp \
    ryml/ext/c4core/src/c4/windows_push.hpp \
    ryml/src/c4/yml/common.hpp \
    ryml/src/c4/yml/detail/checks.hpp \
    ryml/src/c4/yml/detail/parser_dbg.hpp \
    ryml/src/c4/yml/detail/print.hpp \
    ryml/src/c4/yml/detail/stack.hpp \
    ryml/src/c4/yml/emit.def.hpp \
    ryml/src/c4/yml/emit.hpp \
    ryml/src/c4/yml/export.hpp \
    ryml/src/c4/yml/node.hpp \
    ryml/src/c4/yml/parse.hpp \
    ryml/src/c4/yml/preprocess.hpp \
    ryml/src/c4/yml/std/map.hpp \
    ryml/src/c4/yml/std/std.hpp \
    ryml/src/c4/yml/std/string.hpp \
    ryml/src/c4/yml/std/vector.hpp \
    ryml/src/c4/yml/tree.hpp \
    ryml/src/c4/yml/writer.hpp \
    ryml/src/c4/yml/yml.hpp \
    ryml/src/ryml.hpp \
    ryml/src/ryml_std.hpp

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qmenu2.cpp \
    ryml/ext/c4core/src/c4/base64.cpp \
    ryml/ext/c4core/src/c4/char_traits.cpp \
    ryml/ext/c4core/src/c4/error.cpp \
    ryml/ext/c4core/src/c4/format.cpp \
    ryml/ext/c4core/src/c4/language.cpp \
    ryml/ext/c4core/src/c4/memory_resource.cpp \
    ryml/ext/c4core/src/c4/memory_util.cpp \
    ryml/ext/c4core/src/c4/time.cpp \
    ryml/src/c4/yml/common.cpp \
    ryml/src/c4/yml/parse.cpp \
    ryml/src/c4/yml/preprocess.cpp \
    ryml/src/c4/yml/tree.cpp
