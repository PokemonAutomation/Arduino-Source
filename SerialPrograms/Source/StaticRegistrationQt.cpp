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

//  Integrations
#include "Integrations/DiscordIntegrationSettingsWidget.h"

//  Common Tools
#include "CommonTools/Options/QtWidgets/StringSelectWidget.h"
#include "CommonTools/Options/QtWidgets/ScreenWatchWidget.h"
#include "CommonTools/Options/QtWidgets/LanguageOCRWidget.h"

//  Controller Input
#include "ControllerInput/Keyboard/KeyBindingWidget.h"

//  Nintendo Switch
#include "NintendoSwitch/Options/UI/NintendoSwitch_FriendCodeListWidget.h"

//  Pokemon LA
#include "PokemonLA/Options/QtWidgets/PokemonLA_CustomPathTableWidget.h"

namespace PokemonAutomation{



void register_all_statics(){
    //  Basic Options
    RegisterConfigWidget<BatchWidget>();
    RegisterConfigWidget<BooleanCheckBoxCellWidget>();
    RegisterConfigWidget<BooleanCheckBoxOptionWidget>();
    RegisterConfigWidget<BooleanCheckBoxCellWidget>();
    RegisterConfigWidget<BooleanCheckBoxOptionWidget>();
    RegisterConfigWidget<BoxFloatWidget>();
    RegisterConfigWidget<ButtonCellWidget>();
    RegisterConfigWidget<ButtonOptionWidget>();
    RegisterConfigWidget<CheckboxDropdownCellWidget>();
    RegisterConfigWidget<ColorCellWidget>();
    RegisterConfigWidget<DateTimeCellWidget>();
    RegisterConfigWidget<DateTimeOptionWidget>();
    RegisterConfigWidget<EditableTableWidget>();
    RegisterConfigWidget<EnumDropdownCellWidget>();
    RegisterConfigWidget<EnumDropdownOptionWidget>();
    RegisterConfigWidget<FixedCodeWidget>();
    RegisterConfigWidget<FloatingPointCellWidget>();
    RegisterConfigWidget<FloatingPointOptionWidget>();
    RegisterConfigWidget<GroupWidget>();
    RegisterConfigWidget<IntegerRangeCellWidget<uint8_t>>();
    RegisterConfigWidget<MacAddressCellWidget>();
    RegisterConfigWidget<PathCellWidget>();
    RegisterConfigWidget<PathOptionWidget>();
    RegisterConfigWidget<RandomCodeWidget>();
    RegisterConfigWidget<SimpleIntegerCellWidget<uint8_t>>();
    RegisterConfigWidget<SimpleIntegerCellWidget<uint16_t>>();
    RegisterConfigWidget<SimpleIntegerCellWidget<uint32_t>>();
    RegisterConfigWidget<SimpleIntegerCellWidget<uint64_t>>();
    RegisterConfigWidget<SimpleIntegerCellWidget<int8_t>>();
    RegisterConfigWidget<SimpleIntegerCellWidget<int16_t>>();
    RegisterConfigWidget<SimpleIntegerCellWidget<int32_t>>();
    RegisterConfigWidget<SimpleIntegerCellWidget<int64_t>>();
    RegisterConfigWidget<SimpleIntegerOptionWidget<uint8_t>>();
    RegisterConfigWidget<SimpleIntegerOptionWidget<uint16_t>>();
    RegisterConfigWidget<SimpleIntegerOptionWidget<uint32_t>>();
    RegisterConfigWidget<SimpleIntegerOptionWidget<uint64_t>>();
    RegisterConfigWidget<SimpleIntegerOptionWidget<int8_t>>();
    RegisterConfigWidget<SimpleIntegerOptionWidget<int16_t>>();
    RegisterConfigWidget<SimpleIntegerOptionWidget<int32_t>>();
    RegisterConfigWidget<SimpleIntegerOptionWidget<int64_t>>();
    RegisterConfigWidget<StaticTableWidget>();
    RegisterConfigWidget<StaticTextWidget>();
    RegisterConfigWidget<StringCellWidget>();
    RegisterConfigWidget<StringOptionWidget>();
    RegisterConfigWidget<TextEditWidget>();
    RegisterConfigWidget<TimeDurationCellWidget<std::chrono::milliseconds>>();
    RegisterConfigWidget<TimeDurationCellWidget<std::chrono::microseconds>>();
    RegisterConfigWidget<TimeDurationOptionWidget<std::chrono::milliseconds>>();
    RegisterConfigWidget<TimeDurationOptionWidget<std::chrono::microseconds>>();

    //  Common Framework
    RegisterConfigWidget<LabelCellWidget>();
    RegisterConfigWidget<TestButtonWidget>();

    //  Integrations
    RegisterConfigWidget<Integration::DiscordIntegrationSettingsWidget>();

    //  Common Tools
    RegisterConfigWidget<StringSelectCellWidget>();
    RegisterConfigWidget<StringSelectOptionWidget>();
    RegisterConfigWidget<ScreenWatchWidget>();
    RegisterConfigWidget<OCR::LanguageOCRCellWidget>();
    RegisterConfigWidget<OCR::LanguageOCROptionWidget>();

    //  Controller Input
    RegisterConfigWidget<KeyboardHidBindingCellWidget>();

    //  Nintendo Switch
    RegisterConfigWidget<NintendoSwitch::FriendCodeListWidget>();

    //  Pokemon LA
    RegisterConfigWidget<NintendoSwitch::PokemonLA::CustomPathTableWidget>();
}




}
