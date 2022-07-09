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
    ../Common/Cpp/PrettyPrint.cpp \
    ../Common/Cpp/SpinLock.cpp \
    ../Common/Cpp/Unicode.cpp \
    ../Common/NintendoSwitch/NintendoSwitch_Tools.cpp \
    ../Common/Qt/AutoHeightTable.cpp \
    ../Common/Qt/CodeValidator.cpp \
    ../Common/Qt/ExpressionEvaluator.cpp \
    ../Common/Qt/Options/BooleanCheckBox/BooleanCheckBoxBaseOption.cpp \
    ../Common/Qt/Options/BooleanCheckBox/BooleanCheckBoxBaseWidget.cpp \
    ../Common/Qt/Options/EditableTable/EditableTableBaseOption.cpp \
    ../Common/Qt/Options/EditableTable/EditableTableBaseWidget.cpp \
    ../Common/Qt/Options/FloatingPoint/FloatingPointBaseOption.cpp \
    ../Common/Qt/Options/FloatingPoint/FloatingPointBaseWidget.cpp \
    ../Common/Qt/Options/FossilTableBaseOption.cpp \
    ../Common/Qt/Options/MultiHostTableBaseOption.cpp \
    ../Common/Qt/Options/SimpleInteger/SimpleIntegerBaseOption.cpp \
    ../Common/Qt/Options/SimpleInteger/SimpleIntegerBaseWidget.cpp \
    ../Common/Qt/Options/String/StringBaseOption.cpp \
    ../Common/Qt/Options/String/StringBaseWidget.cpp \
    ../Common/Qt/Options/SwitchDate/SwitchDateBaseOption.cpp \
    ../Common/Qt/Options/SwitchDate/SwitchDateBaseWidget.cpp \
    ../Common/Qt/Options/TimeExpression/TimeExpressionBaseOption.cpp \
    ../Common/Qt/Options/TimeExpression/TimeExpressionBaseWidget.cpp \
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
    ../Common/Cpp/Exceptions.h \
    ../Common/Cpp/Json/JsonArray.h \
    ../Common/Cpp/Json/JsonObject.h \
    ../Common/Cpp/Json/JsonTools.h \
    ../Common/Cpp/Json/JsonValue.h \
    ../Common/Cpp/PrettyPrint.h \
    ../Common/Cpp/SpinLock.h \
    ../Common/Cpp/Unicode.h \
    ../Common/NintendoSwitch/NintendoSwitch_Tools.h \
    ../Common/Qt/AutoHeightTable.h \
    ../Common/Qt/CodeValidator.h \
    ../Common/Qt/ExpressionEvaluator.h \
    ../Common/Qt/Options/BooleanCheckBox/BooleanCheckBoxBaseWidget.h \
    ../Common/Qt/Options/BooleanCheckBox/BooleanCheckBoxBaseOption.h \
    ../Common/Qt/Options/EditableTable/EditableTableBaseOption.h \
    ../Common/Qt/Options/EditableTable/EditableTableBaseWidget.h \
    ../Common/Qt/Options/FloatingPoint/FloatingPointBaseOption.h \
    ../Common/Qt/Options/FloatingPoint/FloatingPointBaseWidget.h \
    ../Common/Qt/Options/FossilTableBaseOption.h \
    ../Common/Qt/Options/MultiHostTableBaseOption.h \
    ../Common/Qt/Options/SimpleInteger/SimpleIntegerBaseOption.h \
    ../Common/Qt/Options/SimpleInteger/SimpleIntegerBaseWidget.h \
    ../Common/Qt/Options/String/StringBaseOption.h \
    ../Common/Qt/Options/String/StringBaseWidget.h \
    ../Common/Qt/Options/SwitchDate/SwitchDateBaseOption.h \
    ../Common/Qt/Options/SwitchDate/SwitchDateBaseWidget.h \
    ../Common/Qt/Options/TimeExpression/TimeExpressionBaseOption.h \
    ../Common/Qt/Options/TimeExpression/TimeExpressionBaseWidget.h \
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
