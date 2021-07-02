/*  Pokemon Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/PokemonSwSh/PokemonSettings.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "PokemonSwSh_SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


PokemonSettings_Descriptor::PokemonSettings_Descriptor()
    : PanelDescriptor(
        QColor(),
        "PokemonSwSh:GlobalSettings",
        STRING_POKEMON + " Settings",
        "",
        "Global " + STRING_POKEMON + " Settings"
    )
{}



PokemonSettings::PokemonSettings(const PokemonSettings_Descriptor& descriptor)
    : SettingsPanelInstance(descriptor)
{
    m_options.emplace_back(
        "",
        new SectionDivider("<font size=4><b>Egg Options:</b></font>")
    );
    m_options.emplace_back(
        "AUTO_DEPOSIT",
        new BooleanCheckBox(
            AUTO_DEPOSIT,
            "<b>Auto-Deposit:</b><br>true = Send " + STRING_POKEMON + " to boxes is \"Automatic\".<br>false = Send " + STRING_POKEMON + " to boxes is \"Manual\".",
            true
        )
    );
    m_options.emplace_back(
        "EGG_FETCH_EXTRA_LINE",
        new BooleanCheckBox(
            EGG_FETCH_EXTRA_LINE,
            "<b>Egg Fetch Extra Line:</b><br>true = The daycare lady has an extra line of text in Japanese. Set this to true if you are running any of the egg programs in a Japanese game.",
            false
        )
    );
    m_options.emplace_back(
        "FETCH_EGG_MASH_DELAY",
        new TimeExpression<uint16_t>(
            FETCH_EGG_MASH_DELAY,
            "<b>Fetch Egg Mash Delay:</b><br>Time needed to mash B to fetch an egg and return to overworld when auto-deposit is on.",
            "800"
        )
    );
    m_options.emplace_back(
        "",
        new SectionDivider("<font size=4><b>Den Options:</b></font>")
    );
    m_options.emplace_back(
        "DODGE_UNCATCHABLE_PROMPT_FAST",
        new BooleanCheckBox(
            DODGE_UNCATCHABLE_PROMPT_FAST,
            "<b>Dodge Uncatchable Prompt Fast:</b><br>Which method to use to bypass the uncatchable " + STRING_POKEMON + " prompt?<br>true = Use a fast (but potentially unreliable) method.<br>false = Use a slower (by about 5 seconds) method.",
            false
        )
    );
    m_options.emplace_back(
        "",
        new SectionDivider("<font size=5><b>Advanced Options:</b></font> You shouldn't need to touch anything below here.")
    );
    m_options.emplace_back(
        "",
        new SectionDivider("<font size=4><b>General Timings:</b></font>")
    );
    m_options.emplace_back(
        "AUTO_FR_DURATION",
        new TimeExpression<uint16_t>(
            AUTO_FR_DURATION,
            "<b>Auto-FR Duration:</b><br>Time to accept FRs before returning to den lobby.",
            "8 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "",
        new SectionDivider("<font size=4><b>Menu Navigation Timings:</b></font>")
    );
    m_options.emplace_back(
        "OVERWORLD_TO_MENU_DELAY",
        new TimeExpression<uint16_t>(
            OVERWORLD_TO_MENU_DELAY,
            "<b>Overworld to Menu Delay:</b><br>Delay to bring up the menu when pressing X in the overworld.",
            "120"
        )
    );
    m_options.emplace_back(
        "MENU_TO_OVERWORLD_DELAY",
        new TimeExpression<uint16_t>(
            MENU_TO_OVERWORLD_DELAY,
            "<b>Menu to Overworld Delay:</b><br>Delay to go from menu back to overworld.",
            "250"
        )
    );
    m_options.emplace_back(
        "GAME_TO_HOME_DELAY_FAST",
        new TimeExpression<uint16_t>(
            GAME_TO_HOME_DELAY_FAST,
            "<b>Game to Home Delay (fast):</b><br>Delay from pressing home to entering the the Switch home menu. This affects the speed of date-spamming programs",
            "100"
        )
    );
    m_options.emplace_back(
        "GAME_TO_HOME_DELAY_SAFE",
        new TimeExpression<uint16_t>(
            GAME_TO_HOME_DELAY_SAFE,
            "<b>Game to Home Delay (safe):</b><br>Delay from pressing home to entering the the Switch home menu. This affects the speed of date-spamming programs",
            "125"
        )
    );
    m_options.emplace_back(
        "HOME_TO_GAME_DELAY",
        new TimeExpression<uint16_t>(
            HOME_TO_GAME_DELAY,
            "<b>Home to Game Delay:</b><br>Delay to enter game from home menu.",
            "3 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "OPEN_YCOMM_DELAY",
        new TimeExpression<uint16_t>(
            OPEN_YCOMM_DELAY,
            "<b>Open Y-COMM Delay:</b><br>Time needed to open Y-COMM.",
            "200"
        )
    );
    m_options.emplace_back(
        "ENTER_PROFILE_DELAY",
        new TimeExpression<uint16_t>(
            ENTER_PROFILE_DELAY,
            "<b>Enter Profile Delay:</b><br>Delay to enter your Switch profile.",
            "2 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "",
        new SectionDivider("<font size=4><b>Start Game Timings:</b></font>")
    );
    m_options.emplace_back(
        "START_GAME_MASH",
        new TimeExpression<uint16_t>(
            START_GAME_MASH,
            "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
            "2 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "START_GAME_WAIT",
        new TimeExpression<uint16_t>(
            START_GAME_WAIT,
            "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
            "20 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "ENTER_GAME_MASH",
        new TimeExpression<uint16_t>(
            ENTER_GAME_MASH,
            "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
            "3 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "ENTER_GAME_WAIT",
        new TimeExpression<uint16_t>(
            ENTER_GAME_WAIT,
            "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
            "11 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "",
        new SectionDivider("<font size=4><b>Den Timings:</b></font>")
    );
    m_options.emplace_back(
        "ENTER_OFFLINE_DEN_DELAY",
        new TimeExpression<uint16_t>(
            ENTER_OFFLINE_DEN_DELAY,
            "<b>Enter Offline Game Delay:</b><br>Time needed to enter a den on final button press.",
            "125"
        )
    );
    m_options.emplace_back(
        "REENTER_DEN_DELAY",
        new TimeExpression<uint16_t>(
            REENTER_DEN_DELAY,
            "<b>Re-enter Den Delay:</b><br>Time from exiting den after a day-skip to when you can collect watts and re-enter it.",
            "5 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "COLLECT_WATTS_OFFLINE_DELAY",
        new TimeExpression<uint16_t>(
            COLLECT_WATTS_OFFLINE_DELAY,
            "<b>Collect Watts Delay (offline):</b>",
            "80"
        )
    );
    m_options.emplace_back(
        "COLLECT_WATTS_ONLINE_DELAY",
        new TimeExpression<uint16_t>(
            COLLECT_WATTS_ONLINE_DELAY,
            "<b>Collect Watts Delay (online):</b>",
            "3 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "UNCATCHABLE_PROMPT_DELAY",
        new TimeExpression<uint16_t>(
            UNCATCHABLE_PROMPT_DELAY,
            "<b>Uncatchable Prompt Delay:</b><br>Time needed to bypass uncatchable pokemon prompt.",
            "110"
        )
    );
    m_options.emplace_back(
        "OPEN_LOCAL_DEN_LOBBY_DELAY",
        new TimeExpression<uint16_t>(
            OPEN_LOCAL_DEN_LOBBY_DELAY,
            "<b>Open Local Den Lobby Delay:</b><br>Time needed to open a den lobby on local connection.",
            "3 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "ENTER_SWITCH_POKEMON",
        new TimeExpression<uint16_t>(
            ENTER_SWITCH_POKEMON,
            "<b>Enter Switch " + STRING_POKEMON + ":</b><br>Time needed to enter Switch " + STRING_POKEMON + ".",
            "4 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "EXIT_SWITCH_POKEMON",
        new TimeExpression<uint16_t>(
            EXIT_SWITCH_POKEMON,
            "<b>Exit Switch " + STRING_POKEMON + ":</b><br>Time needed to exit Switch " + STRING_POKEMON + " back to den lobby.",
            "3 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "FULL_LOBBY_TIMER",
        new TimeExpression<uint16_t>(
            FULL_LOBBY_TIMER,
            "<b>Full Lobby Timer:</b><br>Always 3 minutes.",
            "180 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "",
        new SectionDivider("<font size=4><b>Box Timings:</b></font> (for egg programs)")
    );
    m_options.emplace_back(
        "BOX_SCROLL_DELAY",
        new TimeExpression<uint16_t>(
            BOX_SCROLL_DELAY,
            "<b>Box Scroll Delay:</b><br>Delay to move the cursor.",
            "20"
        )
    );
    m_options.emplace_back(
        "BOX_CHANGE_DELAY",
        new TimeExpression<uint16_t>(
            BOX_CHANGE_DELAY,
            "<b>Box Change Delay:</b><br>Delay to change boxes.",
            "80"
        )
    );
    m_options.emplace_back(
        "BOX_PICKUP_DROP_DELAY",
        new TimeExpression<uint16_t>(
            BOX_PICKUP_DROP_DELAY,
            "<b>Box Pickup/Drop Delay:</b><br>Delay to pickup/drop " + STRING_POKEMON + ".",
            "90"
        )
    );
    m_options.emplace_back(
        "MENU_TO_POKEMON_DELAY",
        new TimeExpression<uint16_t>(
            MENU_TO_POKEMON_DELAY,
            "<b>Menu To " + STRING_POKEMON + " Delay:</b><br>Delay to enter " + STRING_POKEMON + " menu.",
            "300"
        )
    );
    m_options.emplace_back(
        "POKEMON_TO_BOX_DELAY",
        new TimeExpression<uint16_t>(
            POKEMON_TO_BOX_DELAY,
            "<b>" + STRING_POKEMON + " to Box Delay:</b><br>Delay to enter box system.",
            "300"
        )
    );
    m_options.emplace_back(
        "BOX_TO_POKEMON_DELAY",
        new TimeExpression<uint16_t>(
            BOX_TO_POKEMON_DELAY,
            "<b>Box to " + STRING_POKEMON + " Delay:</b><br>Delay to exit box system.",
            "250"
        )
    );
    m_options.emplace_back(
        "POKEMON_TO_MENU_DELAY",
        new TimeExpression<uint16_t>(
            POKEMON_TO_MENU_DELAY,
            "<b>" + STRING_POKEMON + " to Menu Delay:</b><br>Delay to return to menu.",
            "250"
        )
    );
}





}
}
}






