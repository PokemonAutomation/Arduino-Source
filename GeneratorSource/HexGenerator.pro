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
    ../Common/Clientside/PrettyPrint.cpp \
    ../Common/Clientside/Unicode.cpp \
    ../Common/Qt/CodeValidator.cpp \
    ../Common/Qt/ExpressionEvaluator.cpp \
    ../Common/Qt/Options/BooleanCheckBoxOption.cpp \
    ../Common/Qt/Options/FloatingPointOption.cpp \
    ../Common/Qt/Options/FossilTableOption.cpp \
    ../Common/Qt/Options/MultiHostTableOption.cpp \
    ../Common/Qt/Options/SimpleIntegerOption.cpp \
	../Common/Qt/Options/StringOption.cpp \
    ../Common/Qt/Options/SwitchDateOption.cpp \
    ../Common/Qt/Options/TimeExpressionOption.cpp \
    ../Common/Qt/QtJsonTools.cpp \
    Source/Main.cpp \
    Source/Options/BooleanCheckBox.cpp \
    Source/Options/ConfigItem.cpp \
    Source/Options/Divider.cpp \
    Source/Options/EnumDropdown.cpp \
    Source/Options/FixedCode.cpp \
    Source/Options/FloatingPoint.cpp \
    Source/Options/FossilTable.cpp \
    Source/Options/MultiHostTable.cpp \
    Source/Options/RandomCode.cpp \
    Source/Options/SimpleInteger.cpp \
    Source/Options/SingleStatementOption.cpp \
    Source/Options/SwitchDate.cpp \
    Source/Options/TimeExpression.cpp \
    Source/Panels/ConfigSet.cpp \
    Source/Panels/JsonProgram.cpp \
    Source/Panels/JsonSettings.cpp \
    Source/Panels/PanelList.cpp \
    Source/Panels/Program.cpp \
    Source/Tools/CommandRunner.cpp \
    Source/Tools/MiscTools.cpp \
    Source/Tools/PersistentSettings.cpp \
    Source/UI/BoardList.cpp \
    Source/UI/MainWindow.cpp \
    Source/UI/ProgramListUI.cpp \
    Source/UI/SettingListUI.cpp

HEADERS += \
    ../Common/Clientside/PrettyPrint.h \
    ../Common/Clientside/Unicode.h \
    ../Common/Qt/CodeValidator.h \
    ../Common/Qt/ExpressionEvaluator.h \
    ../Common/Qt/Options/BooleanCheckBoxOption.h \
    ../Common/Qt/Options/FloatingPointOption.h \
    ../Common/Qt/Options/FossilTableOption.h \
    ../Common/Qt/Options/MultiHostTableOption.h \
    ../Common/Qt/Options/SimpleIntegerOption.h \
	../Common/Qt/Options/StringOption.h \
    ../Common/Qt/Options/SwitchDateOption.h \
    ../Common/Qt/Options/TimeExpressionOption.h \
    ../Common/Qt/QtJsonTools.h \
    ../Common/Qt/StringException.h \
    Source/Options/BooleanCheckBox.h \
    Source/Options/ConfigItem.h \
    Source/Options/Divider.h \
    Source/Options/EnumDropdown.h \
    Source/Options/FixedCode.h \
    Source/Options/FloatingPoint.h \
    Source/Options/FossilTable.h \
    Source/Options/MultiHostTable.h \
    Source/Options/RandomCode.h \
    Source/Options/SimpleInteger.h \
    Source/Options/SingleStatementOption.h \
    Source/Options/SwitchDate.h \
    Source/Options/TimeExpression.h \
    Source/Panels/ConfigSet.h \
    Source/Panels/JsonProgram.h \
    Source/Panels/JsonSettings.h \
    Source/Panels/PanelList.h \
    Source/Panels/Program.h \
    Source/Panels/RightPanel.h \
    Source/Tools/PersistentSettings.h \
    Source/Tools/Tools.h \
    Source/UI/BoardList.h \
    Source/UI/MainWindow.h \
    Source/UI/ProgramListUI.h \
    Source/UI/SettingListUI.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
