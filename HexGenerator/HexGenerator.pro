QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG -= app_bundle
INCLUDEPATH += ../
INCLUDEPATH += Source/

#QMAKE_CFLAGS += -MT
#QMAKE_CFLAGS_RELEASE -= -MD
#QMAKE_CXXFLAGS += -MT
#QMAKE_CXXFLAGS_RELEASE -= -MD

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32-msvc{
    QMAKE_CXXFLAGS += /std:c++latest
}

SOURCES += \
    ../Common/Cpp/Exception.cpp \
    ../Common/Cpp/PrettyPrint.cpp \
    ../Common/Cpp/SpinLock.cpp \
    ../Common/Cpp/Unicode.cpp \
    ../Common/NintendoSwitch/NintendoSwitch_Tools.cpp \
    ../Common/Qt/AutoHeightTable.cpp \
    ../Common/Qt/CodeValidator.cpp \
    ../Common/Qt/ExpressionEvaluator.cpp \
    ../Common/Qt/Options/BooleanCheckBoxOptionBase.cpp \
    ../Common/Qt/Options/EditableTableOptionBase.cpp \
    ../Common/Qt/Options/FloatingPointOptionBase.cpp \
    ../Common/Qt/Options/FossilTableOptionBase.cpp \
    ../Common/Qt/Options/MultiHostTableOptionBase.cpp \
    ../Common/Qt/Options/SimpleIntegerOptionBase.cpp \
    ../Common/Qt/Options/StringOptionBase.cpp \
    ../Common/Qt/Options/SwitchDateOptionBase.cpp \
    ../Common/Qt/Options/TimeExpressionOptionBase.cpp \
    ../Common/Qt/QtJsonTools.cpp \
    Source/Main.cpp \
    Source/Options/BooleanCheckBoxOption.cpp \
    Source/Options/ConfigItem.cpp \
    Source/Options/Divider.cpp \
    Source/Options/EnumDropdownOption.cpp \
    Source/Options/FixedCodeOption.cpp \
    Source/Options/FloatingPointOption.cpp \
    Source/Options/FossilTableOption.cpp \
    Source/Options/MultiHostTableOption.cpp \
    Source/Options/RandomCodeOption.cpp \
    Source/Options/SimpleIntegerOption.cpp \
    Source/Options/SingleStatementOption.cpp \
    Source/Options/SwitchDateOption.cpp \
    Source/Options/TimeExpressionOption.cpp \
    Source/Panels/ConfigSet.cpp \
    Source/Panels/JsonProgram.cpp \
    Source/Panels/JsonSettings.cpp \
    Source/Panels/PanelList.cpp \
    Source/Panels/PanelLists.cpp \
    Source/Panels/Program.cpp \
    Source/Tools/CommandRunner.cpp \
    Source/Tools/MiscTools.cpp \
    Source/Tools/PersistentSettings.cpp \
    Source/UI/BoardList.cpp \
    Source/UI/MainWindow.cpp

HEADERS += \
    ../Common/Cpp/Exception.h \
    ../Common/Cpp/PrettyPrint.h \
    ../Common/Cpp/SpinLock.h \
    ../Common/Cpp/Unicode.h \
    ../Common/NintendoSwitch/NintendoSwitch_Tools.h \
    ../Common/Qt/AutoHeightTable.h \
    ../Common/Qt/CodeValidator.h \
    ../Common/Qt/ExpressionEvaluator.h \
    ../Common/Qt/Options/BooleanCheckBoxOptionBase.h \
    ../Common/Qt/Options/EditableTableOptionBase.h \
    ../Common/Qt/Options/FloatingPointOptionBase.h \
    ../Common/Qt/Options/FossilTableOptionBase.h \
    ../Common/Qt/Options/MultiHostTableOptionBase.h \
    ../Common/Qt/Options/SimpleIntegerOptionBase.h \
    ../Common/Qt/Options/StringOptionBase.h \
    ../Common/Qt/Options/SwitchDateOptionBase.h \
    ../Common/Qt/Options/TimeExpressionOptionBase.h \
    ../Common/Qt/QtJsonTools.h \
    Source/Options/BooleanCheckBoxOption.h \
    Source/Options/ConfigItem.h \
    Source/Options/Divider.h \
    Source/Options/EnumDropdownOption.h \
    Source/Options/FixedCodeOption.h \
    Source/Options/FloatingPointOption.h \
    Source/Options/FossilTableOption.h \
    Source/Options/MultiHostTableOption.h \
    Source/Options/RandomCodeOption.h \
    Source/Options/SimpleIntegerOption.h \
    Source/Options/SingleStatementOption.h \
    Source/Options/SwitchDateOption.h \
    Source/Options/TimeExpressionOption.h \
    Source/Panels/ConfigSet.h \
    Source/Panels/JsonProgram.h \
    Source/Panels/JsonSettings.h \
    Source/Panels/PanelList.h \
    Source/Panels/PanelLists.h \
    Source/Panels/Program.h \
    Source/Panels/RightPanel.h \
    Source/Tools/PersistentSettings.h \
    Source/Tools/Tools.h \
    Source/UI/BoardList.h \
    Source/UI/MainWindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
