/*  Nintendo Switch Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch_Panels.h"

#include "NintendoSwitch_SettingsPanel.h"

#include "GameConsole/Panels/GameConsole_VirtualConsole.h"
#include "GameConsole/Panels/GameConsole_BoxDraw.h"
#include "GameConsole/Panels/GameConsole_SwitchViewer.h"

#include "Programs/NintendoSwitch_TurboA.h"
#include "Programs/NintendoSwitch_TurboButton.h"
#include "Programs/NintendoSwitch_TurboMacro.h"
#include "Programs/NintendoSwitch_PushJoySticks.h"
#include "Programs/NintendoSwitch_PreventSleep.h"
#include "Programs/NintendoSwitch_FriendCodeAdder.h"
#include "Programs/NintendoSwitch_FriendDelete.h"
#include "Programs/NintendoSwitch_RecordKeyboardController.h"

#include "Programs/NintendoSwitch_SnapshotDumper.h"

#include "Programs/NintendoSwitch_MenuStabilityTester.h"
#include "DevPrograms/TestProgramComputer.h"
#include "DevPrograms/TestProgramSwitch.h"
#include "DevPrograms/JoyconProgram.h"
#include "DevPrograms/WaterfillTemplateMaker.h"
#include "DevPrograms/TestDudunsparceFormDetector.h"
#include "Pokemon/Inference/Pokemon_TrainIVCheckerOCR.h"
#include "Pokemon/Inference/Pokemon_TrainPokemonOCR.h"

#include "ComputerPrograms/UnitTestRunner.h"

#ifdef PA_OFFICIAL
#include "../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.h"
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(
        "Nintendo Switch",
        RESOURCE_PATH() + "CategoryIcons/NintendoSwitch.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_OptionsPanel<ConsoleSettingsPanel>());

    ret.emplace_back("---- Virtual Consoles ----");
    ret.emplace_back(GameConsole::make_ConsolePanel<GameConsole::VirtualConsole>());
    ret.emplace_back(GameConsole::make_ConsolePanel<GameConsole::MultiControllerTester>());
    ret.emplace_back(GameConsole::make_MultiConsolePanel<GameConsole::MultiConsoleViewer_Descriptor, GameConsole::MultiConsolePanelInstance>());
//    ret.emplace_back(make_panel<SwitchViewer_Descriptor, SwitchViewer>());

    ret.emplace_back("---- Programs ----");
    ret.emplace_back(make_single_switch_program<TurboA_Descriptor, TurboA>());
    ret.emplace_back(make_single_switch_program<TurboButton_Descriptor, TurboButton>());
    ret.emplace_back(make_single_switch_program<TurboMacro_Descriptor, TurboMacro>());
    ret.emplace_back(make_single_switch_program<PushJoySticks_Descriptor, PushJoySticks>());
    ret.emplace_back(make_single_switch_program<PreventSleep_Descriptor, PreventSleep>());
    ret.emplace_back(make_single_switch_program<FriendCodeAdder_Descriptor, FriendCodeAdder>());
    ret.emplace_back(make_single_switch_program<FriendDelete_Descriptor, FriendDelete>());
    ret.emplace_back(make_single_switch_program<RecordKeyboardController_Descriptor, RecordKeyboardController>());

    ret.emplace_back("---- Testing ----");
    ret.emplace_back(GameConsole::make_ConsolePanel<GameConsole::BoxDraw>());
    ret.emplace_back(make_single_switch_program<SnapshotDumper_Descriptor, SnapshotDumper>());

    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<MenuStabilityTester_Descriptor, MenuStabilityTester>());
        ret.emplace_back(make_ComputerProgram<TestProgramComputer>());
        ret.emplace_back(make_MultiSwitchProgram<TestProgram>());
        ret.emplace_back(make_single_switch_program<JoyconProgram_Descriptor, JoyconProgram>());
        ret.emplace_back(make_single_switch_program<WaterfillTemplateMaker_Descriptor, WaterfillTemplateMaker>());
        ret.emplace_back(make_ComputerProgram<Pokemon::TrainIVCheckerOCR>());
        ret.emplace_back(make_ComputerProgram<Pokemon::TrainPokemonOCR>());
        ret.emplace_back(make_single_switch_program<TestDudunsparceFormDetector_Descriptor, TestDudunsparceFormDetector>());
        ret.emplace_back(make_ComputerProgram<ComputerPrograms::UnitTestRunner>());
#ifdef PA_OFFICIAL
        if (STATIC_GLOBALS.INTERNAL_DEVELOPER_MODE){
            add_panels(ret);
        }
#endif
    }

    return ret;
}





}
}
