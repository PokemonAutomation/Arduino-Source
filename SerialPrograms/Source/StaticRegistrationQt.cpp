/*  Static Registration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Options/QtWidgets/ScreenWatchWidget.h"
#include "Common/Qt/Options/BatchWidget.h"

//  Basic Options
#include "Common/Qt/Options/BooleanCheckBoxWidget.h"
#include "Common/Qt/Options/BoxFloatWidget.h"
#include "Common/Qt/Options/ButtonWidget.h"
#include "Common/Qt/Options/CheckboxDropdownWidget.h"
#include "Common/Qt/Options/ColorWidget.h"
#include "Common/Qt/Options/DateWidget.h"
//#include "Common/Qt/Options/ConfigWidget.h"
#include "Common/Qt/Options/EditableTableWidget.h"
#include "Common/Qt/Options/EnumDropdownWidget.h"
#include "Common/Qt/Options/FixedCodeWidget.h"
#include "Common/Qt/Options/FloatingPointWidget.h"
#include "Common/Qt/Options/GroupWidget.h"
#include "Common/Qt/Options/IntegerRangeWidget.h"
#include "Common/Qt/Options/MacAddressWidget.h"
#include "Common/Qt/Options/PathWidget.h"
#include "Common/Qt/Options/RandomCodeWidget.h"
#include "Common/Qt/Options/SimpleIntegerWidget.h"
#include "Common/Qt/Options/StaticTableWidget.h"
#include "Common/Qt/Options/StaticTextWidget.h"
#include "Common/Qt/Options/StringWidget.h"
#include "Common/Qt/Options/TextEditWidget.h"
#include "Common/Qt/Options/TimeDurationWidget.h"
#include "CommonFramework/Options/QtWidget/LabelCellWidget.h"

//  Common Framework
#include "CommonFramework/Notifications/EventNotificationWidget.h"
#include "CommonFramework/Panels/UI/PanelWidget.h"
#include "CommonFramework/Panels/UI/OptionsPanelWidget.h"
#include "CommonFramework/ResourceDownload/SettingsResourceDownloadWidget.h"

// Resource Download
#include "CommonTools/Options/QtWidgets/StringSelectWidget.h"

//  Common Tools
#include "CommonTools/Options/QtWidgets/ScreenWatchWidget.h"
#include "CommonTools/Options/QtWidgets/LanguageOCRWidget.h"
#include "ComputerPrograms/Framework/ComputerProgramWidget.h"

//  Computer Programs
#include "ControllerInput/Keyboard/KeyBindingWidget.h"

//  Controllers
//#include "Controllers/NullControllerWidget.h"
#include "Controllers/ControllerSelectorWidget.h"
#include "Controllers/PABotBase2/SerialPABotBase2_SelectorWidget.h"
#include "GameConsole/UI/ConsoleSystemWidget.h"

//  Consoles
#include "GameConsole/UI/ConsolePanelWidget.h"
#include "GameConsole/UI/ConsoleProgramWidget.h"
#include "GameConsole/UI/MultiConsoleSystemWidget.h"
#include "GameConsole/UI/MultiConsolePanelWidget.h"
#include "GameConsole/UI/MultiConsoleProgramWidget.h"
#include "NintendoSwitch/Controllers/SysbotBase/SysbotBase_SelectorWidget.h"

//  Nintendo Switch
#include "VideoGames/PokemonLA/Options/QtWidgets/PokemonLA_CustomPathTableWidget.h"
//#include "NintendoSwitch/Framework/UI/NintendoSwitch_SingleSwitchProgramWidget.h"

//  Pokemon LA
#include "StaticRegistration.h"

namespace PokemonAutomation{



void register_all_statics(){
    //  Basic Options
    RegisterUiStateQtWidget<BatchWidget>();
    RegisterUiStateQtWidget<BooleanCheckBoxCellWidget>();
    RegisterUiStateQtWidget<BooleanCheckBoxOptionWidget>();
    RegisterUiStateQtWidget<BooleanCheckBoxCellWidget>();
    RegisterUiStateQtWidget<BooleanCheckBoxOptionWidget>();
    RegisterUiStateQtWidget<BoxFloatWidget>();
    RegisterUiStateQtWidget<ButtonCellWidget>();
    RegisterUiStateQtWidget<ButtonOptionWidget>();
    RegisterUiStateQtWidget<CheckboxDropdownCellWidget>();
    RegisterUiStateQtWidget<ColorCellWidget>();
    RegisterUiStateQtWidget<ColorOptionWidget>();
    RegisterUiStateQtWidget<DateTimeCellWidget>();
    RegisterUiStateQtWidget<DateTimeOptionWidget>();
    RegisterUiStateQtWidget<EditableTableWidget>();
    RegisterUiStateQtWidget<EnumDropdownCellWidget>();
    RegisterUiStateQtWidget<EnumDropdownOptionWidget>();
    RegisterUiStateQtWidget<FixedCodeWidget>();
    RegisterUiStateQtWidget<FloatingPointCellWidget>();
    RegisterUiStateQtWidget<FloatingPointOptionWidget>();
    RegisterUiStateQtWidget<GroupWidget>();
    RegisterUiStateQtWidget<IntegerRangeCellWidget<uint8_t>>();
    RegisterUiStateQtWidget<MacAddressCellWidget>();
    RegisterUiStateQtWidget<PathCellWidget>();
    RegisterUiStateQtWidget<PathOptionWidget>();
    RegisterUiStateQtWidget<RandomCodeWidget>();
    RegisterUiStateQtWidget<SimpleIntegerCellWidget>();
    RegisterUiStateQtWidget<SimpleIntegerOptionWidget>();
    RegisterUiStateQtWidget<StaticTableWidget>();
    RegisterUiStateQtWidget<StaticTextWidget>();
    RegisterUiStateQtWidget<StringCellWidget>();
    RegisterUiStateQtWidget<StringOptionWidget>();
    RegisterUiStateQtWidget<TextEditWidget>();
    RegisterUiStateQtWidget<TimeDurationCellWidget<std::chrono::milliseconds>>();
    RegisterUiStateQtWidget<TimeDurationCellWidget<std::chrono::microseconds>>();
    RegisterUiStateQtWidget<TimeDurationOptionWidget<std::chrono::milliseconds>>();
    RegisterUiStateQtWidget<TimeDurationOptionWidget<std::chrono::microseconds>>();

    //  Common Framework
    RegisterUiStateQtWidget<LabelCellWidget>();
    RegisterUiStateQtWidget<TestButtonWidget>();
    RegisterUiStateQtWidget<PanelWidget>();
    RegisterUiStateQtWidget<OptionsPanelWidget>();

    // Resource Download
    RegisterUiStateQtWidget<SettingsDownloadButtonWidget>();
    RegisterUiStateQtWidget<SettingsDeleteButtonWidget>();
    RegisterUiStateQtWidget<SettingsCancelButtonWidget>();
    RegisterUiStateQtWidget<SettingsProgressBarWidget>();
    RegisterUiStateQtWidget<SettingsDownloadErrorWidget>();

    //  Common Tools
    RegisterUiStateQtWidget<StringSelectCellWidget>();
    RegisterUiStateQtWidget<StringSelectOptionWidget>();
    RegisterUiStateQtWidget<ScreenWatchWidget>();
    RegisterUiStateQtWidget<OCR::LanguageOCRCellWidget>();
    RegisterUiStateQtWidget<OCR::LanguageOCROptionWidget>();

    //  Computer Programs
    RegisterUiStateQtWidget<ComputerProgramWidget>();

    //  Controllers
//    RegisterUiStateQtWidget<NullControllerWidget>();
    RegisterUiStateQtWidget<ControllerSelectorWidget>();
    RegisterUiStateQtWidget<SerialPABotBase::SerialPABotBase2_SelectorWidget>();
    RegisterUiStateQtWidget<KeyboardHidBindingCellWidget>();

    //  Consoles
    RegisterUiStateQtWidget<GameConsole::ConsoleSystemWidget>();
    RegisterUiStateQtWidget<GameConsole::ConsolePanelWidget>();
    RegisterUiStateQtWidget<GameConsole::ConsoleProgramWidget>();
    RegisterUiStateQtWidget<GameConsole::MultiConsoleSystemWidget>();
    RegisterUiStateQtWidget<GameConsole::MultiConsolePanelWidget>();
    RegisterUiStateQtWidget<GameConsole::MultiConsoleProgramWidget>();

    //  Nintendo Switch
    RegisterUiStateQtWidget<SysbotBase::TcpSysbotBase_SelectorWidget>();

    //  Pokemon LA
    RegisterUiStateQtWidget<NintendoSwitch::PokemonLA::CustomPathTableWidget>();
}




}
