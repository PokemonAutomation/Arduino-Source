/*  Nintendo Switch Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "NintendoSwitch_Panels.h"

#include "NintendoSwitch_Settings.h"

#include "Programs/NintendoSwitch_VirtualConsole.h"
#include "Programs/NintendoSwitch_SwitchViewer.h"

#include "Programs/NintendoSwitch_TurboA.h"
#include "Programs/NintendoSwitch_TurboButton.h"
#include "Programs/NintendoSwitch_PreventSleep.h"
#include "Programs/NintendoSwitch_FriendCodeAdder.h"
#include "Programs/NintendoSwitch_FriendDelete.h"

#include "Programs/PokemonHome_PageSwap.h"

#include "TestProgramComputer.h"
#include "TestProgramSwitch.h"
#include "TestProgramSwitchAudio.h"
#include "NintendoSwitch/InferenceTraining/PokemonHome_GenerateNameOCR.h"
#include "Pokemon/Inference/Pokemon_TrainIVCheckerOCR.h"
#include "Pokemon/Inference/Pokemon_TrainPokemonOCR.h"


namespace PokemonAutomation{
namespace NintendoSwitch{


Panels::Panels(QTabWidget& parent, PanelListener& listener)
    : PanelList(parent, "Switch", listener)
{
    add_divider("---- Settings ----");
    add_settings<ConsoleSettings_Descriptor, ConsoleSettingsPanel>();

    add_divider("---- Virtual Consoles ----");
    add_program<VirtualConsole_Descriptor, VirtualConsole>();
    add_program<SwitchViewer_Descriptor, SwitchViewer>();

    add_divider("---- Programs ----");
    add_program<TurboA_Descriptor, TurboA>();
    add_program<TurboButton_Descriptor, TurboButton>();
    add_program<PreventSleep_Descriptor, PreventSleep>();
    add_program<FriendCodeAdder_Descriptor, FriendCodeAdder>();
    add_program<FriendDelete_Descriptor, FriendDelete>();

//    add_divider("---- " + STRING_POKEMON + " Home ----");
    add_program<PokemonHome::PageSwap_Descriptor, PokemonHome::PageSwap>();

    if (GlobalSettings::instance().DEVELOPER_MODE){
        add_divider("---- Developer Tools ----");
        add_program<TestProgramComputer_Descriptor, TestProgramComputer>();
        add_program<TestProgram_Descriptor, TestProgram>();
        add_program<TestProgramAudio_Descriptor, TestProgramAudio>();
        add_program<PokemonHome::GenerateNameOCRData_Descriptor, PokemonHome::GenerateNameOCRData>();
        add_program<Pokemon::TrainIVCheckerOCR_Descriptor, Pokemon::TrainIVCheckerOCR>();
        add_program<Pokemon::TrainPokemonOCR_Descriptor, Pokemon::TrainPokemonOCR>();
    }


    finish_panel_setup();
}




}
}
