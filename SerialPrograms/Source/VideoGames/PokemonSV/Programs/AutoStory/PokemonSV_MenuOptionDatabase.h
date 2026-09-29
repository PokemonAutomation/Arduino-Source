/*  Menu Option Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MenuOptionDatabase_H
#define PokemonAutomation_PokemonSV_MenuOptionDatabase_H

#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


enum class MenuOptionItemEnum{
    TEXT_SPEED,
    SKIP_MOVE_LEARNING,
    SEND_TO_BOXES,
    GIVE_NICKNAMES,
    VERTICAL_CAMERA_CONTROLS,
    HORIZONTAL_CAMERA_CONTROLS,
    CAMERA_SUPPORT,
    CAMERA_INTERPOLATION,
    CAMERA_DISTANCE,
    AUTOSAVE,
    SHOW_NICKNAMES,
    SKIP_CUTSCENES,
    BACKGROUN_MUSIC,
    SOUND_EFFECTS,
    POKEMON_CRIES,
    CONTROLLER_RUMBLE,
    HELPING_FUNCTIONS,
    CONTROLS_WHILE_FLYING,
};

enum class MenuOptionToggleEnum{
    SLOW,
    AVERAGE,
    NORMAL,
    FAST,
    ON,
    OFF,
    MANUAL,
    AUTOMATIC,
    REGULAR,
    INVERTED,
    CLOSE,
    FAR,
    DONT_SHOW,
    SHOW,
    
};

struct MenuOptionItem{
    MenuOptionItemEnum enum_value;
    std::string slug;
    int8_t index;
};

struct MenuOptionToggle{
    MenuOptionToggleEnum enum_value;
    std::string slug;

};

const MenuOptionItem& menu_option_item_lookup_by_enum(MenuOptionItemEnum enum_value);

const MenuOptionItem& menu_option_item_lookup_by_slug(std::string& slug);

const MenuOptionToggle& menu_option_toggle_lookup_by_enum(MenuOptionToggleEnum enum_value);

const MenuOptionToggle& menu_option_toggle_lookup_by_slug(std::string& slug);

}
}
}
#endif
