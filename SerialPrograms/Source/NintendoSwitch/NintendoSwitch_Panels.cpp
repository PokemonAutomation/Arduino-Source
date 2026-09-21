/*  Nintendo Switch Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "NintendoSwitch_Panels.h"

#include "NintendoSwitch_SettingsPanel.h"

#include "GameConsole/Panels/GameConsole_VirtualConsole.h"
#include "GameConsole/Panels/GameConsole_MultiConsoleViewer.h"
#include "GameConsole/Panels/GameConsole_BoxDraw.h"
#include "GameConsole/Panels/GameConsole_SnapshotDumper.h"

#include "Programs/NintendoSwitch_TurboA.h"
#include "Programs/NintendoSwitch_TurboButton.h"
#include "Programs/NintendoSwitch_TurboMacro.h"
#include "Programs/NintendoSwitch_PushJoySticks.h"
#include "Programs/NintendoSwitch_PreventSleep.h"
#include "Programs/NintendoSwitch_FriendCodeAdder.h"
#include "Programs/NintendoSwitch_FriendDelete.h"
#include "Programs/NintendoSwitch_RecordKeyboardController.h"

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
    ret.emplace_back(GameConsole::make_MultiConsolePanel<GameConsole::MultiConsoleViewer>());
//    ret.emplace_back(make_panel<SwitchViewer_Descriptor, SwitchViewer>());

    ret.emplace_back("---- Programs ----");
    ret.emplace_back(make_SingleSwitchProgram<TurboA>());
    ret.emplace_back(make_SingleSwitchProgram<TurboButton>());
    ret.emplace_back(make_SingleSwitchProgram<TurboMacro>());
    ret.emplace_back(make_SingleSwitchProgram<PushJoySticks>());
    ret.emplace_back(make_SingleSwitchProgram<PreventSleep>());
    ret.emplace_back(make_SingleSwitchProgram<FriendCodeAdder>());
    ret.emplace_back(make_SingleSwitchProgram<FriendDelete>());
    ret.emplace_back(make_SingleSwitchProgram<RecordKeyboardController>());

    ret.emplace_back("---- Testing ----");
    ret.emplace_back(GameConsole::make_ConsolePanel<GameConsole::BoxDraw>());
    ret.emplace_back(GameConsole::make_ConsoleProgram<GameConsole::SnapshotDumper>());

    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_SingleSwitchProgram<MenuStabilityTester>());
        ret.emplace_back(make_ComputerProgram<TestProgramComputer>());
        ret.emplace_back(make_MultiSwitchProgram<TestProgram>());
        ret.emplace_back(make_SingleSwitchProgram<JoyconProgram>());
        ret.emplace_back(make_SingleSwitchProgram<WaterfillTemplateMaker>());
        ret.emplace_back(make_ComputerProgram<Pokemon::TrainIVCheckerOCR>());
        ret.emplace_back(make_ComputerProgram<Pokemon::TrainPokemonOCR>());
        ret.emplace_back(make_SingleSwitchProgram<TestDudunsparceFormDetector>());
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
