QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
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
    ../Common/Cpp/Concurrency/SpinLock.cpp \
    ../Common/Cpp/Containers/AlignedMalloc.cpp \
    ../Common/Cpp/EnumDatabase.cpp \
    ../Common/Cpp/Exceptions.cpp \
    ../Common/Cpp/Json/JsonArray.cpp \
    ../Common/Cpp/Json/JsonObject.cpp \
    ../Common/Cpp/Json/JsonTools.cpp \
    ../Common/Cpp/Json/JsonValue.cpp \
    ../Common/Cpp/LifetimeSanitizer.cpp \
    ../Common/Cpp/Options/BooleanCheckBoxOption.cpp \
    ../Common/Cpp/Options/ConfigOption.cpp \
    ../Common/Cpp/Options/DateOption.cpp \
    ../Common/Cpp/Options/EditableTableOption2.cpp \
    ../Common/Cpp/Options/EnumDropdownOption.cpp \
    ../Common/Cpp/Options/FloatingPointOption.cpp \
    ../Common/Cpp/Options/SimpleIntegerOption.cpp \
    ../Common/Cpp/Options/TimeExpressionOption.cpp \
    ../Common/Cpp/PrettyPrint.cpp \
    ../Common/Cpp/Unicode.cpp \
    ../Common/Qt/AutoHeightTable.cpp \
    ../Common/Qt/CodeValidator.cpp \
    ../Common/Qt/ExpressionEvaluator.cpp \
    ../Common/Qt/Options/BooleanCheckBoxWidget.cpp \
    ../Common/Qt/Options/ConfigWidget.cpp \
    ../Common/Qt/Options/DateWidget.cpp \
    ../Common/Qt/Options/EditableTable/EditableTableBaseOption.cpp \
    ../Common/Qt/Options/EditableTable/EditableTableBaseWidget.cpp \
    ../Common/Qt/Options/EditableTableWidget2.cpp \
    ../Common/Qt/Options/EnumDropdownWidget.cpp \
    ../Common/Qt/Options/FloatingPointWidget.cpp \
    ../Common/Qt/Options/SimpleIntegerWidget.cpp \
    ../Common/Qt/Options/TimeExpressionWidget.cpp \
    Source/Main.cpp \
    Source/Options/BooleanCheckBox.cpp \
    Source/Options/ConfigItem.cpp \
    Source/Options/Date.cpp \
    Source/Options/Divider.cpp \
    Source/Options/EnumDropdown.cpp \
    Source/Options/FixedCodeOption.cpp \
    Source/Options/FloatingPoint.cpp \
    Source/Options/FossilTableOption.cpp \
    Source/Options/MultiHostTable.cpp \
    Source/Options/RandomCodeOption.cpp \
    Source/Options/SimpleInteger.cpp \
    Source/Options/SingleStatementOption.cpp \
    Source/Options/TimeExpression.cpp \
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
    ../Common/Cpp/Containers/AlignedMalloc.h \
    ../Common/Cpp/Containers/Pimpl.h \
    ../Common/Cpp/Containers/Pimpl.tpp \
    ../Common/Cpp/EnumDatabase.h \
    ../Common/Cpp/Exceptions.h \
    ../Common/Cpp/Json/JsonArray.h \
    ../Common/Cpp/Json/JsonObject.h \
    ../Common/Cpp/Json/JsonTools.h \
    ../Common/Cpp/Json/JsonValue.h \
    ../Common/Cpp/LifetimeSanitizer.h \
    ../Common/Cpp/Options/BooleanCheckBoxOption.h \
    ../Common/Cpp/Options/ConfigOption.h \
    ../Common/Cpp/Options/DateOption.h \
    ../Common/Cpp/Options/EditableTableOption2.h \
    ../Common/Cpp/Options/EnumDropdownOption.h \
    ../Common/Cpp/Options/FloatingPointOption.h \
    ../Common/Cpp/Options/SimpleIntegerOption.h \
    ../Common/Cpp/Options/TimeExpressionOption.h \
    ../Common/Cpp/PrettyPrint.h \
    ../Common/Cpp/Unicode.h \
    ../Common/NintendoSwitch/NintendoSwitch_SlotDatabase.h \
    ../Common/PokemonSwSh/PokemonSwSh_FossilTable.h \
    ../Common/PokemonSwSh/PokemonSwSh_MultiHostTable.h \
    ../Common/Qt/AutoHeightTable.h \
    ../Common/Qt/CodeValidator.h \
    ../Common/Qt/ExpressionEvaluator.h \
    ../Common/Qt/Options/BooleanCheckBoxWidget.h \
    ../Common/Qt/Options/ConfigWidget.h \
    ../Common/Qt/Options/DateWidget.h \
    ../Common/Qt/Options/EditableTable/EditableTableBaseOption.h \
    ../Common/Qt/Options/EditableTable/EditableTableBaseWidget.h \
    ../Common/Qt/Options/EditableTableWidget2.h \
    ../Common/Qt/Options/EnumDropdownWidget.h \
    ../Common/Qt/Options/FloatingPointWidget.h \
    ../Common/Qt/Options/SimpleIntegerWidget.h \
    ../Common/Qt/Options/TimeExpressionWidget.h \
    Source/Options/BooleanCheckBox.h \
    Source/Options/ConfigItem.h \
    Source/Options/Date.h \
    Source/Options/Divider.h \
    Source/Options/EnumDropdown.h \
    Source/Options/FixedCodeOption.h \
    Source/Options/FloatingPoint.h \
    Source/Options/FossilTableOption.h \
    Source/Options/MultiHostTable.h \
    Source/Options/RandomCodeOption.h \
    Source/Options/SimpleInteger.h \
    Source/Options/SingleStatementOption.h \
    Source/Options/TimeExpression.h \
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
