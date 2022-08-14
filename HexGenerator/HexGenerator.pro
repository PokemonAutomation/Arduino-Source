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
    ../Common/Cpp/Exceptions.cpp \
    ../Common/Cpp/Json/JsonArray.cpp \
    ../Common/Cpp/Json/JsonObject.cpp \
    ../Common/Cpp/Json/JsonTools.cpp \
    ../Common/Cpp/Json/JsonValue.cpp \
    ../Common/Cpp/Options/BooleanCheckBoxOption.cpp \
    ../Common/Cpp/Options/ConfigOption.cpp \
    ../Common/Cpp/Options/FloatingPointOption.cpp \
    ../Common/Cpp/Options/SimpleIntegerOption.cpp \
    ../Common/Cpp/PrettyPrint.cpp \
    ../Common/Cpp/SpinLock.cpp \
    ../Common/Cpp/Unicode.cpp \
    ../Common/NintendoSwitch/NintendoSwitch_Tools.cpp \
    ../Common/Qt/AutoHeightTable.cpp \
    ../Common/Qt/CodeValidator.cpp \
    ../Common/Qt/ExpressionEvaluator.cpp \
    ../Common/Qt/Options/BooleanCheckBoxWidget.cpp \
    ../Common/Qt/Options/ConfigWidget.cpp \
    ../Common/Qt/Options/EditableTable/EditableTableBaseOption.cpp \
    ../Common/Qt/Options/EditableTable/EditableTableBaseWidget.cpp \
    ../Common/Qt/Options/FloatingPointWidget.cpp \
    ../Common/Qt/Options/FossilTableBaseOption.cpp \
    ../Common/Qt/Options/MultiHostTableBaseOption.cpp \
    ../Common/Qt/Options/SimpleIntegerWidget.cpp \
    ../Common/Qt/Options/SwitchDate/SwitchDateBaseOption.cpp \
    ../Common/Qt/Options/SwitchDate/SwitchDateBaseWidget.cpp \
    ../Common/Qt/Options/TimeExpression/TimeExpressionBaseOption.cpp \
    ../Common/Qt/Options/TimeExpression/TimeExpressionBaseWidget.cpp \
    Source/Main.cpp \
    Source/Options/BooleanCheckBox.cpp \
    Source/Options/ConfigItem.cpp \
    Source/Options/Divider.cpp \
    Source/Options/EnumDropdownOption.cpp \
    Source/Options/FixedCodeOption.cpp \
    Source/Options/FossilTableOption.cpp \
    Source/Options/MultiHostTableOption.cpp \
    Source/Options/RandomCodeOption.cpp \
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
    ../Common/Cpp/Exceptions.h \
    ../Common/Cpp/Json/JsonArray.h \
    ../Common/Cpp/Json/JsonObject.h \
    ../Common/Cpp/Json/JsonTools.h \
    ../Common/Cpp/Json/JsonValue.h \
    ../Common/Cpp/Options/BooleanCheckBoxOption.h \
    ../Common/Cpp/Options/ConfigOption.h \
    ../Common/Cpp/Options/FloatingPointOption.h \
    ../Common/Cpp/Options/SimpleIntegerOption.h \
    ../Common/Cpp/PrettyPrint.h \
    ../Common/Cpp/SpinLock.h \
    ../Common/Cpp/Unicode.h \
    ../Common/NintendoSwitch/NintendoSwitch_Tools.h \
    ../Common/Qt/AutoHeightTable.h \
    ../Common/Qt/CodeValidator.h \
    ../Common/Qt/ExpressionEvaluator.h \
    ../Common/Qt/Options/BooleanCheckBoxWidget.h \
    ../Common/Qt/Options/ConfigWidget.h \
    ../Common/Qt/Options/EditableTable/EditableTableBaseOption.h \
    ../Common/Qt/Options/EditableTable/EditableTableBaseWidget.h \
    ../Common/Qt/Options/FloatingPointOption.h \
    ../Common/Qt/Options/FloatingPointWidget.h \
    ../Common/Qt/Options/FossilTableBaseOption.h \
    ../Common/Qt/Options/MultiHostTableBaseOption.h \
    ../Common/Qt/Options/SimpleIntegerOption.h \
    ../Common/Qt/Options/SimpleIntegerWidget.h \
    ../Common/Qt/Options/SwitchDate/SwitchDateBaseOption.h \
    ../Common/Qt/Options/SwitchDate/SwitchDateBaseWidget.h \
    ../Common/Qt/Options/TimeExpression/TimeExpressionBaseOption.h \
    ../Common/Qt/Options/TimeExpression/TimeExpressionBaseWidget.h \
    Source/Options/BooleanCheckBox.h \
    Source/Options/ConfigItem.h \
    Source/Options/Divider.h \
    Source/Options/EnumDropdownOption.h \
    Source/Options/FixedCodeOption.h \
    Source/Options/FloatingPoint.h \
    Source/Options/FossilTableOption.h \
    Source/Options/MultiHostTableOption.h \
    Source/Options/RandomCodeOption.h \
    Source/Options/SimpleInteger.h \
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
