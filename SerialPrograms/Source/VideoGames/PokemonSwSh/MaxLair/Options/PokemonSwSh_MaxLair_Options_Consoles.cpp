/*  Max Lair Consoles
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh_MaxLair_Options_Consoles.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{

using namespace Pokemon;


CaughtScreenActionOption::CaughtScreenActionOption(
    bool take_non_shiny, bool reset_if_high_winrate,
    std::string label, CaughtScreenAction default_action
)
    : EnumDropdownOption<CaughtScreenAction>(
        std::move(label),
        {
            {CaughtScreenAction::STOP_PROGRAM, "stop-program", "Stop Program"},
            {CaughtScreenAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE, "continue", take_non_shiny
                ? "Continue Running. (Take any shiny non-boss " + STRING_POKEMON + " along the way.)"
                : "Continue Running"
            },
            {CaughtScreenAction::RESET, "reset", !reset_if_high_winrate
                ? "Reset Game"
                : take_non_shiny
                    ? "Reset Game if win-rate is above the threshold. Otherwise take any non-boss shinies and continue."
                    : "Reset Game if win-rate is above the threshold. Otherwise continue running."
            },
        },
        LockMode::LOCK_WHILE_RUNNING,
        default_action
    )
{}

CaughtScreenActionsOption::CaughtScreenActionsOption(
    bool host_tooltip, bool winrate_reset_tooltip,
    CaughtScreenAction default_no_shinies,
    CaughtScreenAction default_shiny_nonboss,
    CaughtScreenAction default_shiny_boss
)
    : GroupOption("End Adventure Actions", LockMode::LOCK_WHILE_RUNNING)
    , no_shinies(false, winrate_reset_tooltip, "<b>No Shinies:</b>", default_no_shinies)
    , shiny_nonboss(
        true, winrate_reset_tooltip,
        "<b>Boss is not shiny, but something else is:</b><br>"
        "If this is set to continue and there are multiple shinies, the program will take the highest one on the list.",
        default_shiny_nonboss
    )
    , shiny_boss(
        true, winrate_reset_tooltip,
        "<b>Boss/Legendary is Shiny:</b><br>If there are multiple shinies where one is the boss, this option still applies.<br><br>"
        "<font color=\"red\">For safety reasons, this program will <i>NEVER</i> automatically take a boss. "
        "Likewise, the settings here are intentionally worded so that it's impossible to ask the program to take a boss. "
        "Taking a boss is irreversible, therefore we refuse to automate it. We require you to do it manually to avoid any mistakes.<br><br>"
        "<b>Please DO NOT ask us to implement the ability to automatically take a boss. Thank you.</b></font>",
        default_shiny_boss
    )
    , description(
        std::string("Choosing \"Reset Game\" has the effect of preserving your balls at the cost of paying ore. (10 ore per reset after enough resets) "
        "Therefore, you should start with plenty of ore to avoid running out.") +
        (
            host_tooltip
                ? "<br>If this is the hosting Switch, \"Reset Game\" also has the effect of preserving the path. "
                "So if you have a path with a high win rate, set both \"No Shinies\" and \"Shiny Non-Boss\" to \"Reset Game\" to grind for the boss."
                : ""
        )
    )
{
    PA_ADD_OPTION(no_shinies);
    PA_ADD_OPTION(shiny_nonboss);
    PA_ADD_OPTION(shiny_boss);
    PA_ADD_STATIC(description);
}





ConsoleSpecificOptions::ConsoleSpecificOptions(std::string label, const LanguageSet& languages, bool host)
    : GroupOption(std::move(label), LockMode::LOCK_WHILE_RUNNING)
    , is_host_label("<font color=\"blue\" size=4><b>This is the host Switch.</b></font>")
    , language("<b>Game Language:</b>", languages, LockMode::LOCK_WHILE_RUNNING, true)
{
    ConsoleSpecificOptions::set_host(host);
    PA_ADD_STATIC(is_host_label);
    PA_ADD_OPTION(language);
}
void ConsoleSpecificOptions::set_host(bool is_host){
    this->is_host = is_host;
    is_host_label.set_visibility(is_host ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
}





Consoles::~Consoles(){
    HOST.remove_listener(*this);
}
Consoles::Consoles(const ConsoleSpecificOptionsFactory& factory)
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_languages(PokemonNameReader::instance().languages())
{
    PLAYERS[0] = factory.make("Switch 0 (Top Left)", m_languages, true);
    PLAYERS[1] = factory.make("Switch 1 (Top Right)", m_languages, false);
    PLAYERS[2] = factory.make("Switch 2 (Bottom Left)", m_languages, false);
    PLAYERS[3] = factory.make("Switch 3 (Bottom Right)", m_languages, false);

    PA_ADD_OPTION(HOST);
    add_option(*PLAYERS[0], "Console0");
    add_option(*PLAYERS[1], "Console1");
    add_option(*PLAYERS[2], "Console2");
    add_option(*PLAYERS[3], "Console3");

    set_active_consoles(1);

    HOST.add_listener(*this);
}
size_t Consoles::active_consoles() const{
    return m_active_consoles;
}
void Consoles::set_active_consoles(size_t consoles){
//    cout << "Consoles::set_active_consoles() = " << consoles << endl;
    size_t c = 0;
    for (; c < consoles; c++){
        PLAYERS[c]->set_visibility(ConfigOptionState::ENABLED);
    }
    for (; c < 4; c++){
        PLAYERS[c]->set_visibility(ConfigOptionState::HIDDEN);
    }
    m_active_consoles = consoles;
}
void Consoles::on_config_value_changed(void* object){
    size_t host_index = HOST.current_value();
    for (size_t c = 0; c < 4; c++){
        PLAYERS[c]->set_host(c == host_index);
    }
}





}
}
}
}
