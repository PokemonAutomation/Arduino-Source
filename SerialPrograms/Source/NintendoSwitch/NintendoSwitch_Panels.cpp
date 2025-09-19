/*  Nintendo Switch Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch_Panels.h"

#include "NintendoSwitch_Settings.h"

#include "Programs/NintendoSwitch_VirtualConsole.h"
#include "Programs/NintendoSwitch_SwitchViewer.h"

#include "Programs/NintendoSwitch_TurboA.h"
#include "Programs/NintendoSwitch_TurboButton.h"
#include "Programs/NintendoSwitch_TurboMacro.h"
#include "Programs/NintendoSwitch_PushJoySticks.h"
#include "Programs/NintendoSwitch_PreventSleep.h"
#include "Programs/NintendoSwitch_FriendCodeAdder.h"
#include "Programs/NintendoSwitch_FriendDelete.h"
#include "Programs/NintendoSwitch_RecordKeyboardController.h"

#include "DevPrograms/BoxDraw.h"
#include "Programs/NintendoSwitch_SnapshotDumper.h"
#include "Programs/NintendoSwitch_MenuStabilityTester.h"
#include "DevPrograms/TestProgramComputer.h"
#include "DevPrograms/TestProgramSwitch.h"
#include "DevPrograms/JoyconProgram.h"
#include "DevPrograms/TestDudunsparceFormDetector.h"
#include "Pokemon/Inference/Pokemon_TrainIVCheckerOCR.h"
#include "Pokemon/Inference/Pokemon_TrainPokemonOCR.h"

#ifdef PA_OFFICIAL
#include "../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.h"
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor("Nintendo Switch")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<ConsoleSettings_Descriptor, ConsoleSettingsPanel>());

    ret.emplace_back("---- Virtual Consoles ----");
    ret.emplace_back(make_panel<VirtualConsole_Descriptor, VirtualConsole>());
    ret.emplace_back(make_panel<SwitchViewer_Descriptor, SwitchViewer>());

    ret.emplace_back("---- Programs ----");
    ret.emplace_back(make_single_switch_program<TurboA_Descriptor, TurboA>());
    ret.emplace_back(make_single_switch_program<TurboButton_Descriptor, TurboButton>());
    ret.emplace_back(make_single_switch_program<TurboMacro_Descriptor, TurboMacro>());
    ret.emplace_back(make_single_switch_program<PushJoySticks_Descriptor, PushJoySticks>());
    ret.emplace_back(make_single_switch_program<PreventSleep_Descriptor, PreventSleep>());
    ret.emplace_back(make_single_switch_program<FriendCodeAdder_Descriptor, FriendCodeAdder>());
    ret.emplace_back(make_single_switch_program<FriendDelete_Descriptor, FriendDelete>());
    ret.emplace_back(make_single_switch_program<RecordKeyboardController_Descriptor, RecordKeyboardController>());

//    ret.emplace_back("---- " + STRING_POKEMON + " Home ----");

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<BoxDraw_Descriptor, BoxDraw>());
        ret.emplace_back(make_single_switch_program<SnapshotDumper_Descriptor, SnapshotDumper>());
        ret.emplace_back(make_single_switch_program<MenuStabilityTester_Descriptor, MenuStabilityTester>());
        ret.emplace_back(make_computer_program<TestProgramComputer_Descriptor, TestProgramComputer>());
        ret.emplace_back(make_multi_switch_program<TestProgram_Descriptor, TestProgram>());
        ret.emplace_back(make_single_switch_program<JoyconProgram_Descriptor, JoyconProgram>());
        ret.emplace_back(make_computer_program<Pokemon::TrainIVCheckerOCR_Descriptor, Pokemon::TrainIVCheckerOCR>());
        ret.emplace_back(make_computer_program<Pokemon::TrainPokemonOCR_Descriptor, Pokemon::TrainPokemonOCR>());
        ret.emplace_back(make_single_switch_program<TestDudunsparceFormDetector_Descriptor, TestDudunsparceFormDetector>());
#ifdef PA_OFFICIAL
        add_panels(ret);
#endif
    }

    return ret;
}





}
}
