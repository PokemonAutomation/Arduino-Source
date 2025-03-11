/*  Zelda: TotK Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ZeldaTotK_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
    , GAME_TO_HOME_DELAY0(
        "<b>Game to Home Delay:</b><br>Delay from pressing home to entering the the Switch home menu.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_MASH0(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , START_GAME_WAIT0(
        "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
        LockMode::LOCK_WHILE_RUNNING,
        "60 s"
    )
    , ENTER_GAME_MASH0(
        "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , ENTER_GAME_WAIT0(
        "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "60 s"
    )
{
    PA_ADD_STATIC(m_menu_navigation);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY0);
    PA_ADD_STATIC(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_MASH0);
    PA_ADD_OPTION(START_GAME_WAIT0);
    PA_ADD_OPTION(ENTER_GAME_MASH0);
    PA_ADD_OPTION(ENTER_GAME_WAIT0);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "ZeldaTotK:GlobalSettings",
        "Zelda: TotK", "Tears of the Kingdom Settings",
        "ComputerControl/blob/master/Wiki/Programs/ZeldaTotK/TotKSettings.md",
        "Global Zelda: Tears of the Kingom Settings"
    )
{}



GameSettingsPanel::GameSettingsPanel(const GameSettings_Descriptor& descriptor)
    : SettingsPanelInstance(descriptor)
    , settings(GameSettings::instance())
{
    PA_ADD_OPTION(settings);
}





}
}
}






