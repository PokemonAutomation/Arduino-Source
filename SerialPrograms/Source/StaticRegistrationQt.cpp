/*  Static Registration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Options/QtWidgets/ScreenWatchWidget.h"
#include "StaticRegistration.h"

//  Basic Options
#include "Common/Qt/Options/BatchWidget.h"
#include "Common/Qt/Options/BooleanCheckBoxWidget.h"
#include "Common/Qt/Options/BoxFloatWidget.h"
#include "Common/Qt/Options/ButtonWidget.h"
#include "Common/Qt/Options/CheckboxDropdownWidget.h"
#include "Common/Qt/Options/ColorWidget.h"
//#include "Common/Qt/Options/ConfigWidget.h"
#include "Common/Qt/Options/DateWidget.h"
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

//  Common Framework
#include "CommonFramework/Options/QtWidget/LabelCellWidget.h"
#include "CommonFramework/Notifications/EventNotificationWidget.h"

// Resource Download
#include "CommonFramework/ResourceDownload/SettingsResourceDownloadWidget.h"

//  Common Tools
#include "CommonTools/Options/QtWidgets/StringSelectWidget.h"
#include "CommonTools/Options/QtWidgets/ScreenWatchWidget.h"
#include "CommonTools/Options/QtWidgets/LanguageOCRWidget.h"

//  Controllers
#include "Controllers/NullControllerWidget.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_SelectorWidget.h"
#include "Controllers/PABotBase2/SerialPABotBase2_SelectorWidget.h"
#include "ControllerInput/Keyboard/KeyBindingWidget.h"

//  Nintendo Switch
#include "NintendoSwitch/Controllers/SysbotBase/SysbotBase_SelectorWidget.h"
#include "NintendoSwitch/Options/UI/NintendoSwitch_FriendCodeListWidget.h"

//  Pokemon LA
#include "PokemonLA/Options/QtWidgets/PokemonLA_CustomPathTableWidget.h"

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

    //  Controllers
    RegisterUiStateQtWidget<NullControllerWidget>();
    RegisterUiStateQtWidget<SerialPABotBase::SerialPABotBase_SelectorWidget>();
    RegisterUiStateQtWidget<SerialPABotBase::SerialPABotBase2_SelectorWidget>();
    RegisterUiStateQtWidget<KeyboardHidBindingCellWidget>();

    //  Nintendo Switch
    RegisterUiStateQtWidget<SysbotBase::TcpSysbotBase_SelectorWidget>();
    RegisterUiStateQtWidget<NintendoSwitch::FriendCodeListWidget>();

    //  Pokemon LA
    RegisterUiStateQtWidget<NintendoSwitch::PokemonLA::CustomPathTableWidget>();
}




}
