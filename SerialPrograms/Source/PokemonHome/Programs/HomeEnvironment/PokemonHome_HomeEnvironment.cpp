#include "PokemonHome_HomeEnvironment.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

// Alias for the callable type
using NavigationFunction = std::function<void(SingleSwitchProgramEnvironment&, ProControllerContext&, const std::string&)>;

// Alias for the unordered_map type
using NavigationMap = std::unordered_map<PageID, std::vector<std::pair<PageID, NavigationFunction>>>;


HomeCursor::HomeCursor(SingleSwitchProgramEnvironment& env, ProControllerContext&){

}

void HomeCursor::locate_position(){
    // TODO: Implement locate_position logic
}

void HomeCursor::move_cursor_to(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::pair<size_t, size_t>& cursor){
    // TODO: Implement cursor movement logic
}

void HomeCursor::pick_up_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // TODO: Implement pick up logic
}

void HomeCursor::put_down_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // TODO: Implement put down logic
}



PokemonHome_HomeEnvironment::PokemonHome_HomeEnvironment(SingleSwitchProgramEnvironment& env, ProControllerContext& context)
    : cursor(env, context)
{
    DetectHome(env, context);
}

void PokemonHome_HomeEnvironment::DetectHome(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

}

void PokemonHome_HomeEnvironment::initialize_navigation_map(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    navigation_map[PageID::TITLE_SCREEN] = {
        {PageID::MAIN_MENU,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
            // Press A
        }},
    };

    navigation_map[PageID::MAIN_MENU] = {
        {PageID::GAME_SELECTION,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
            // Press A
        }},
    };

    navigation_map[PageID::GAME_SELECTION] = {
        {PageID::MAIN_MENU,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
            // Press B
        }},
        {PageID::BOX_VIEW,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
            // Cycle through games until we see the correct game on screen, then press A and wait for login.
        }},
    };

    navigation_map[PageID::BOX_VIEW] = {
        {PageID::MAIN_MENU,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
            // Press Plus, then trigger logout sequence
        }},
        {PageID::SUMMARY_VIEW,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
            // Open menu, then go to summary. Assumes cursor is in the right position.
        }},
        {PageID::MARKINGS_VIEW,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
             // Open menu, then go to markings. Assumes cursor is in the right position.
        }},
        {PageID::LIST_VIEW,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
            // Press Y
        }},
    };

    navigation_map[PageID::SUMMARY_VIEW] = {
        {PageID::BOX_VIEW,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
             // Press B
         }},
    };

    navigation_map[PageID::MARKINGS_VIEW] = {
        {PageID::BOX_VIEW,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
             // Press B
         }},
    };

    navigation_map[PageID::LIST_VIEW] = {
        {PageID::BOX_VIEW,
         [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const GameStatus) {
             // Press B
         }},
    };
}

}
}
}
