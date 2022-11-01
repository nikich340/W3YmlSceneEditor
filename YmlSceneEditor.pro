QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS_WARN_OFF += -Wunused-parameter -Wunused-variable

# External files
RESOURCES = \
    data/YmlSceneEditor.qrc

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
    yaml-cpp/include \
    yaml-cpp/src \
    UI \
    SectionsEditor \
    ShotEditor

HEADERS += \
    SectionsEditor/EdgeItem.h \
    SectionsEditor/GraphicsSectionItem.h \
    MainWindow.h \
    SectionsEditor/SectionsGraphicsView.h \
    SectionsEditor/SocketItem.h \
    ShotEditor/ShotManager.h \
    ShotEditor/ShotScrollArea.h \
    SimpleTrie.h \
    UI/CustomRectItem.h \
    UI/DialogChangeSection.h \
    UI/InputDialogshot.h \
    UI/InputMimicPose.h \
    UI/InputPlacement.h \
    UI/InputPose.h \
    UI/RepoActorsDialog.h \
    UI/SocketSettingsForm.h \
    YmlSceneManager.h \
    YmlStructs.h \
    constants.h \
    yaml-cpp/include/yaml-cpp/anchor.h \
    yaml-cpp/include/yaml-cpp/binary.h \
    yaml-cpp/include/yaml-cpp/contrib/anchordict.h \
    yaml-cpp/include/yaml-cpp/contrib/graphbuilder.h \
    yaml-cpp/include/yaml-cpp/depthguard.h \
    yaml-cpp/include/yaml-cpp/dll.h \
    yaml-cpp/include/yaml-cpp/emitfromevents.h \
    yaml-cpp/include/yaml-cpp/emitter.h \
    yaml-cpp/include/yaml-cpp/emitterdef.h \
    yaml-cpp/include/yaml-cpp/emittermanip.h \
    yaml-cpp/include/yaml-cpp/emitterstyle.h \
    yaml-cpp/include/yaml-cpp/eventhandler.h \
    yaml-cpp/include/yaml-cpp/exceptions.h \
    yaml-cpp/include/yaml-cpp/mark.h \
    yaml-cpp/include/yaml-cpp/node/convert.h \
    yaml-cpp/include/yaml-cpp/node/detail/impl.h \
    yaml-cpp/include/yaml-cpp/node/detail/iterator.h \
    yaml-cpp/include/yaml-cpp/node/detail/iterator_fwd.h \
    yaml-cpp/include/yaml-cpp/node/detail/memory.h \
    yaml-cpp/include/yaml-cpp/node/detail/node.h \
    yaml-cpp/include/yaml-cpp/node/detail/node_data.h \
    yaml-cpp/include/yaml-cpp/node/detail/node_iterator.h \
    yaml-cpp/include/yaml-cpp/node/detail/node_ref.h \
    yaml-cpp/include/yaml-cpp/node/emit.h \
    yaml-cpp/include/yaml-cpp/node/impl.h \
    yaml-cpp/include/yaml-cpp/node/iterator.h \
    yaml-cpp/include/yaml-cpp/node/node.h \
    yaml-cpp/include/yaml-cpp/node/parse.h \
    yaml-cpp/include/yaml-cpp/node/ptr.h \
    yaml-cpp/include/yaml-cpp/node/type.h \
    yaml-cpp/include/yaml-cpp/noexcept.h \
    yaml-cpp/include/yaml-cpp/null.h \
    yaml-cpp/include/yaml-cpp/ostream_wrapper.h \
    yaml-cpp/include/yaml-cpp/parser.h \
    yaml-cpp/include/yaml-cpp/stlemitter.h \
    yaml-cpp/include/yaml-cpp/traits.h \
    yaml-cpp/include/yaml-cpp/yaml.h \
    yaml-cpp/src/collectionstack.h \
    yaml-cpp/src/contrib/graphbuilderadapter.h \
    yaml-cpp/src/directives.h \
    yaml-cpp/src/emitterstate.h \
    yaml-cpp/src/emitterutils.h \
    yaml-cpp/src/exp.h \
    yaml-cpp/src/indentation.h \
    yaml-cpp/src/nodebuilder.h \
    yaml-cpp/src/nodeevents.h \
    yaml-cpp/src/ptr_vector.h \
    yaml-cpp/src/regex_yaml.h \
    yaml-cpp/src/regeximpl.h \
    yaml-cpp/src/scanner.h \
    yaml-cpp/src/scanscalar.h \
    yaml-cpp/src/scantag.h \
    yaml-cpp/src/setting.h \
    yaml-cpp/src/singledocparser.h \
    yaml-cpp/src/stream.h \
    yaml-cpp/src/streamcharsource.h \
    yaml-cpp/src/stringsource.h \
    yaml-cpp/src/tag.h \
    yaml-cpp/src/token.h

FORMS += \
    UI/DialogChangeSection.ui \
    UI/InputDialogshot.ui \
    UI/InputMimicPose.ui \
    UI/InputPlacement.ui \
    UI/InputPose.ui \
    UI/RepoActorsDialog.ui \
    UI/SocketSettingsForm.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

SOURCES += \
    SectionsEditor/EdgeItem.cpp \
    SectionsEditor/GraphicsSectionItem.cpp \
    MainWindow.cpp \
    SectionsEditor/SectionsGraphicsView.cpp \
    SectionsEditor/SocketItem.cpp \
    ShotEditor/ShotManager.cpp \
    ShotEditor/ShotScrollArea.cpp \
    UI/CustomRectItem.cpp \
    UI/DialogChangeSection.cpp \
    UI/InputDialogshot.cpp \
    UI/InputMimicPose.cpp \
    UI/InputPlacement.cpp \
    UI/InputPose.cpp \
    UI/RepoActorsDialog.cpp \
    UI/SocketSettingsForm.cpp \
    YmlSceneManager.cpp \
    constants.cpp \
    main.cpp \
    yaml-cpp/src/binary.cpp \
    yaml-cpp/src/contrib/graphbuilder.cpp \
    yaml-cpp/src/contrib/graphbuilderadapter.cpp \
    yaml-cpp/src/convert.cpp \
    yaml-cpp/src/depthguard.cpp \
    yaml-cpp/src/directives.cpp \
    yaml-cpp/src/emit.cpp \
    yaml-cpp/src/emitfromevents.cpp \
    yaml-cpp/src/emitter.cpp \
    yaml-cpp/src/emitterstate.cpp \
    yaml-cpp/src/emitterutils.cpp \
    yaml-cpp/src/exceptions.cpp \
    yaml-cpp/src/exp.cpp \
    yaml-cpp/src/memory.cpp \
    yaml-cpp/src/node.cpp \
    yaml-cpp/src/node_data.cpp \
    yaml-cpp/src/nodebuilder.cpp \
    yaml-cpp/src/nodeevents.cpp \
    yaml-cpp/src/null.cpp \
    yaml-cpp/src/ostream_wrapper.cpp \
    yaml-cpp/src/parse.cpp \
    yaml-cpp/src/parser.cpp \
    yaml-cpp/src/regex_yaml.cpp \
    yaml-cpp/src/scanner.cpp \
    yaml-cpp/src/scanscalar.cpp \
    yaml-cpp/src/scantag.cpp \
    yaml-cpp/src/scantoken.cpp \
    yaml-cpp/src/simplekey.cpp \
    yaml-cpp/src/singledocparser.cpp \
    yaml-cpp/src/stream.cpp \
    yaml-cpp/src/tag.cpp

DISTFILES +=
