/*  Nintendo Switch Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "Panels_NintendoSwitch.h"

#include "FrameworkSettingsPanel.h"

#include "Programs/VirtualConsole.h"
#include "Programs/SwitchViewer.h"

#include "Programs/TurboButton.h"
#include "Programs/PreventSleep.h"
#include "Programs/FriendCodeAdder.h"
#include "Programs/FriendDelete.h"

#include "Programs/PokemonHome_PageSwap.h"

#include "TestProgram.h"
#include "NintendoSwitch/InferenceTraining/PokemonHome_GenerateNameOCR.h"
#include "Pokemon/Inference/Pokemon_TrainIVCheckerOCR.h"
#include "Pokemon/Inference/Pokemon_TrainPokemonOCR.h"


namespace PokemonAutomation{
namespace NintendoSwitch{


Panels::Panels(QTabWidget& parent, PanelListener& listener)
    : PanelList(parent, "Switch", listener)
{
    PersistentSettings& settings = PERSISTENT_SETTINGS();

    add_divider("---- Settings ----");
    add_settings<FrameworkSettings_Descriptor, FrameworkSettings>();

    add_divider("---- Virtual Consoles ----");
    add_program<VirtualConsole_Descriptor, VirtualConsole>();
    add_program<SwitchViewer_Descriptor, SwitchViewer>();

    add_divider("---- Programs ----");
    add_program<TurboButton_Descriptor, TurboButton>();
    add_program<PreventSleep_Descriptor, PreventSleep>();
    add_program<FriendCodeAdder_Descriptor, FriendCodeAdder>();
    add_program<FriendDelete_Descriptor, FriendDelete>();

//    add_divider("---- " + STRING_POKEMON + " Home ----");
    add_program<PokemonHome::PageSwap_Descriptor, PokemonHome::PageSwap>();

    if (settings.developer_mode){
        add_divider("---- Developer Tools ----");
        add_program<TestProgram_Descriptor, TestProgram>();
        add_program<PokemonHome::GenerateNameOCRData_Descriptor, PokemonHome::GenerateNameOCRData>();
        add_program<Pokemon::TrainIVCheckerOCR_Descriptor, Pokemon::TrainIVCheckerOCR>();
        add_program<Pokemon::TrainPokemonOCR_Descriptor, Pokemon::TrainPokemonOCR>();
    }


    finish_panel_setup();
}




}
}
