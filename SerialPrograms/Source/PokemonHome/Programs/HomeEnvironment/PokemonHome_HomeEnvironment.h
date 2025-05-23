#ifndef PokemonAutomation_PokemonHome_HomeEnvironment_H
#define PokemonAutomation_PokemonHome_HomeEnvironment_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonHome/Options/PokemonHome_BoxSortingTable.h"
#include <functional>
#include <unordered_map>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonHome {

enum class PageID {
    TITLE_SCREEN,
    MAIN_MENU,
    GAME_SELECTION,
    BOX_VIEW,
    SUMMARY_VIEW,
    MARKINGS_VIEW,
    LIST_VIEW,
    UNKNOWN
};

}
}
}

namespace std {
template <>
struct hash<PokemonAutomation::NintendoSwitch::PokemonHome::PageID> {
    std::size_t operator()(const PokemonAutomation::NintendoSwitch::PokemonHome::PageID& id) const noexcept {
        return std::hash<int>()(static_cast<int>(id));
    }
};
}

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonHome {

// Continue with other enums, classes, and declarations
using NavigationFunction = ::std::function<void(SingleSwitchProgramEnvironment&, ProControllerContext&, const std::string&)>;

enum class GameStatus {
    NONE,
    POKEMON_HOME,
    POKEMON_PLA,
    POKEMON_PIKACHU,
    POKEMON_EEVEE,
    POKEMON_DIAMOND,
    POKEMON_PEARL,
    POKEMON_SWORD,
    POKEMON_SHIELD,
    POKEMON_SCARLET,
    POKEMON_VIOLET,
    CURRENT,
    UNKNOWN
};

enum class SecondaryBoxStatus {
    FALSE,
    TRUE,
    UNKNOWN
};

class HomeCursor {
    const size_t MAX_ROWS = 5;
    const size_t MAX_COLUMNS = 6;

public:
    HomeCursor(SingleSwitchProgramEnvironment&, ProControllerContext&);

    void locate_position();
    void move_cursor_to(SingleSwitchProgramEnvironment&, ProControllerContext&, const std::pair<size_t, size_t>);
    void pick_up_pokemon(SingleSwitchProgramEnvironment&, ProControllerContext&);
    void put_down_pokemon(SingleSwitchProgramEnvironment&, ProControllerContext&);

private:
    size_t row;
    size_t column;

    bool InSecondaryBoxes;
};

class PokemonHome_HomeEnvironment : public SingleSwitchProgramInstance {

public:
    PokemonHome_HomeEnvironment(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void navigate_to(SingleSwitchProgramEnvironment&, ProControllerContext&, const GameStatus, const std::pair<size_t, size_t>, const size_t);
    void DetectHome(SingleSwitchProgramEnvironment&, ProControllerContext&);

private:
    void initialize_navigation_map(SingleSwitchProgramEnvironment&, ProControllerContext&);
    PageID find_navigation_path(SingleSwitchProgramEnvironment&, ProControllerContext&, PageID, PageID, const GameStatus);
    void perform_navigation_step(SingleSwitchProgramEnvironment&, ProControllerContext&, PageID, PageID, const GameStatus);

    HomeCursor cursor;
    GameStatus GameOpen;
    PageID current_view;

    std::unordered_map<PageID, std::vector<std::pair<PageID, NavigationFunction>>> navigation_map;
};

}
}
}
#endif
