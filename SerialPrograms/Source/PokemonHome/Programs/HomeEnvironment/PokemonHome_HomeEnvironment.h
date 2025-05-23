#ifndef PokemonAutomation_PokemonHome_HomeEnvironment_H
#define PokemonAutomation_PokemonHome_HomeEnvironment_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

enum class PageID {
    TILE_SCREEN,
    MAIN_MENU,
    GAME_SELECTION,
    BOX_VIEW,
    SUMMARY_VIEW,
    MARKINGS_VIEW,
    LIST_VIEW,
    UNKNOWN
    // etc
};

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
    UNKNOWN
};


class Cursor{
    const size_t MAX_ROWS = 5;
    const size_t MAX_COLUMNS = 6;

public:
    Cursor(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void locate_position();
    void move_cursor_to(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::pair<size_t, size_t>& cursor);
    void pick_up_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void put_down_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    size_t row;
    size_t column;

    bool InSecondaryBoxes;
};


class PokemonHome_HomeEnvironment : public SingleSwitchProgramInstance{

public:
    PokemonHome_HomeEnvironment(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void DetectHome(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    Cursor cursor;
    GameStatus GameOpen;
    PageID current_view;

};

}
}
}
#endif // POKEMONHOME_HOMEENVIRONMENT_H
