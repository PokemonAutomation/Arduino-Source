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
    // Title screen to game selection
    navigation_map[PageID::TITLE_SCREEN] = {
        {PageID::MAIN_MENU, [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::string&) {
            // Press A
        }},
    };

    // Game selection to Main View Base
    navigation_map[PageID::GAME_SELECTION] = {
        {PageID::MAIN_VIEW_BASE, [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::string& game) {
             std::cout << "Selecting game: " << game << " and entering Main View Base.\n";
         }},
    };

    // Main View Base to other pages
    navigation_map[PageID::MAIN_VIEW_BASE] = {
        {PageID::LIST_VIEW, [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::string&) {
             std::cout << "Navigating to List View from Main View Base.\n";
         }},
        {PageID::MAIN_VIEW_OPTION_MENU, [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::string&) {
             std::cout << "Opening Option Menu in Main View.\n";
         }},
    };

    // Main View Option Menu to Summary or Markings
    navigation_map[PageID::MAIN_VIEW_OPTION_MENU] = {
        {PageID::SUMMARY_VIEW, [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::string&) {
             std::cout << "Navigating to Summary View from Option Menu.\n";
         }},
        {PageID::MARKINGS_VIEW, [](SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::string&) {
             std::cout << "Navigating to Markings View from Option Menu.\n";
         }},
    };
}

}
}
}
