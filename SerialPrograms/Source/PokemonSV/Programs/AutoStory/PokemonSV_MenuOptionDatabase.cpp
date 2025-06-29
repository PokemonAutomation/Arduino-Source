/*  Menu Option Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include <map>
#include "Common/Cpp/Exceptions.h"
#include "PokemonSV_MenuOptionDatabase.h"
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


static const std::vector<MenuOptionItem>& MenuOption_AllItems(){
    static const std::vector<MenuOptionItem> database{
        {MenuOptionItemEnum::TEXT_SPEED,                    "text-speed", 0},
        {MenuOptionItemEnum::SKIP_MOVE_LEARNING,            "skip-move-learning", 1},
        {MenuOptionItemEnum::SEND_TO_BOXES,                 "send-to-boxes", 2},
        {MenuOptionItemEnum::GIVE_NICKNAMES,                "give-nicknames", 3},
        {MenuOptionItemEnum::VERTICAL_CAMERA_CONTROLS,      "vertical-camera-controls", 4},
        {MenuOptionItemEnum::HORIZONTAL_CAMERA_CONTROLS,    "horizontal-camera-controls", 5},
        {MenuOptionItemEnum::CAMERA_SUPPORT,                "camera-support", 6},
        {MenuOptionItemEnum::CAMERA_INTERPOLATION,          "camera-interpolation", 7},
        {MenuOptionItemEnum::CAMERA_DISTANCE,               "camera-distance", 8},
        {MenuOptionItemEnum::AUTOSAVE,                      "autosave", 9},
        {MenuOptionItemEnum::SHOW_NICKNAMES,                "show-nicknames", 10},
        {MenuOptionItemEnum::SKIP_CUTSCENES,                "skip-cutscenes", 11},
        {MenuOptionItemEnum::BACKGROUN_MUSIC,               "background-music", 12},
        {MenuOptionItemEnum::SOUND_EFFECTS,                 "sound-effects", 13},
        {MenuOptionItemEnum::POKEMON_CRIES,                 "pokemon-cries", 14},
        {MenuOptionItemEnum::CONTROLLER_RUMBLE,             "controller-rumble", 15},
        {MenuOptionItemEnum::HELPING_FUNCTIONS,             "helping-functions", 16},
        {MenuOptionItemEnum::CONTROLS_WHILE_FLYING,         "controls-while-flying", 17},
    };

    return database;

}

static const std::vector<MenuOptionToggle>& MenuOption_AllToggles(){
    static const std::vector<MenuOptionToggle> database{
        {MenuOptionToggleEnum::SLOW,        "slow"},
        {MenuOptionToggleEnum::AVERAGE,        "average"},
        {MenuOptionToggleEnum::NORMAL,      "normal"},
        {MenuOptionToggleEnum::FAST,        "fast"},
        {MenuOptionToggleEnum::ON,          "on"},
        {MenuOptionToggleEnum::OFF,         "off"},
        {MenuOptionToggleEnum::MANUAL,      "manual"},
        {MenuOptionToggleEnum::AUTOMATIC,   "automatic"},
        {MenuOptionToggleEnum::REGULAR,     "regular"},
        {MenuOptionToggleEnum::INVERTED,    "inverted"},
        {MenuOptionToggleEnum::CLOSE,       "close"},
        {MenuOptionToggleEnum::FAR,         "far"},
        {MenuOptionToggleEnum::DONT_SHOW,   "dont-show"},
        {MenuOptionToggleEnum::SHOW,        "show"},
    };

    return database;

}

static std::map<MenuOptionItemEnum, const MenuOptionItem*> make_MenuOptionItem_lookup_by_enum(){
    std::map<MenuOptionItemEnum, const MenuOptionItem*> ret;
    for (const MenuOptionItem& item : MenuOption_AllItems()){
        if (!ret.emplace(item.enum_value, &item).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum: " + std::to_string((int)item.enum_value));
        }
    }
    return ret;
}
const MenuOptionItem& menu_option_item_lookup_by_enum(MenuOptionItemEnum enum_value){
    static const std::map<MenuOptionItemEnum, const MenuOptionItem*> database = make_MenuOptionItem_lookup_by_enum();
    auto iter = database.find(enum_value);
    if (iter != database.end()){
        return *iter->second;
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Enum: " + std::to_string((int)enum_value));
    }
}
static std::map<std::string, const MenuOptionItem*> make_MenuOptionItem_lookup_by_slug(){
    std::map<std::string, const MenuOptionItem*> ret;
    for (const MenuOptionItem& item : MenuOption_AllItems()){
        if (!ret.emplace(item.slug, &item).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum: " + item.slug);
        }
    }
    return ret;
}
const MenuOptionItem& menu_option_item_lookup_by_slug(std::string& slug){
    static const std::map<std::string, const MenuOptionItem*> database = make_MenuOptionItem_lookup_by_slug();
    auto iter = database.find(slug);
    if (iter != database.end()){
        return *iter->second;
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Enum: " + slug);
    }
}

static std::map<MenuOptionToggleEnum, const MenuOptionToggle*> make_MenuOptionToggle_lookup_by_enum(){
    std::map<MenuOptionToggleEnum, const MenuOptionToggle*> ret;
    for (const MenuOptionToggle& item : MenuOption_AllToggles()){
        if (!ret.emplace(item.enum_value, &item).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum: " + std::to_string((int)item.enum_value));
        }
    }
    return ret;
}
const MenuOptionToggle& menu_option_toggle_lookup_by_enum(MenuOptionToggleEnum enum_value){
    static const std::map<MenuOptionToggleEnum, const MenuOptionToggle*> database = make_MenuOptionToggle_lookup_by_enum();
    auto iter = database.find(enum_value);
    if (iter != database.end()){
        return *iter->second;
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Enum: " + std::to_string((int)enum_value));
    }
}

static std::map<std::string, const MenuOptionToggle*> make_MenuOptionToggle_lookup_by_slug(){
    std::map<std::string, const MenuOptionToggle*> ret;
    for (const MenuOptionToggle& item : MenuOption_AllToggles()){
        if (!ret.emplace(item.slug, &item).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum: " + item.slug);
        }
    }
    return ret;
}
const MenuOptionToggle& menu_option_toggle_lookup_by_slug(std::string& slug){
    static const std::map<std::string, const MenuOptionToggle*> database = make_MenuOptionToggle_lookup_by_slug();
    auto iter = database.find(slug);
    if (iter != database.end()){
        return *iter->second;
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Enum: " + slug);
    }
}




}
}
}
