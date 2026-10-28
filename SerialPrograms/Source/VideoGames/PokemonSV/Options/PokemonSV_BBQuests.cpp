/*  Blueberry Quests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "PokemonSV_BBQuests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

namespace{

const std::map<std::string, BBQuests>& BBQuests_TOKEN_TO_ENUM(){
    static std::map<std::string, BBQuests> data{
        {"auto-10",             BBQuests::auto_10},
        {"make-tm",             BBQuests::make_tm },
        {"pickup-10",           BBQuests::pickup_10},
        {"sneak-up",            BBQuests::sneak_up},
        {"photo-fly",           BBQuests::photo_fly},
        {"photo-swim",          BBQuests::photo_swim},
        {"photo-canyon",        BBQuests::photo_canyon},
        {"photo-coastal",       BBQuests::photo_coastal},
        {"photo-polar",         BBQuests::photo_polar},
        {"photo-savanna",       BBQuests::photo_savanna},
        {"tera-self-defeat",    BBQuests::tera_self_defeat},
        {"travel-500",          BBQuests::travel_500},
        {"catch-any",           BBQuests::catch_any},
        {"catch-normal",        BBQuests::catch_normal},
        {"catch-fighting",      BBQuests::catch_fighting},
        {"catch-flying",        BBQuests::catch_flying},
        {"catch-poison",        BBQuests::catch_poison},
        {"catch-ground",        BBQuests::catch_ground},
        {"catch-rock",          BBQuests::catch_rock},
        {"catch-bug",           BBQuests::catch_bug},
        {"catch-ghost",         BBQuests::catch_ghost},
        {"catch-steel",         BBQuests::catch_steel},
        {"catch-fire",          BBQuests::catch_fire},
        {"catch-water",         BBQuests::catch_water},
        {"catch-grass",         BBQuests::catch_grass},
        {"catch-electric",      BBQuests::catch_electric},
        {"catch-psychic",       BBQuests::catch_psychic},
        {"catch-ice",           BBQuests::catch_ice},
        {"catch-dragon",        BBQuests::catch_dragon},
        {"catch-dark",          BBQuests::catch_dark},
        {"catch-fairy",         BBQuests::catch_fairy},
        {"wash-pokemon",        BBQuests::wash_pokemon},
        {"wild-tera",           BBQuests::wild_tera},
        {"auto-30",             BBQuests::auto_30},
        {"tera-raid",           BBQuests::tera_raid},
        {"sandwich-three",      BBQuests::sandwich_three},
        {"bitter-sandwich",     BBQuests::bitter_sandwich},
        {"sweet-sandwich",      BBQuests::sweet_sandwich},
        {"salty-sandwich",      BBQuests::salty_sandwich},
        {"sour-sandwich",       BBQuests::sour_sandwich},
        {"spicy-sandwich",      BBQuests::spicy_sandwich},
        {"hatch-egg",           BBQuests::hatch_egg},
        {"photo-normal",        BBQuests::photo_normal},
        {"photo-fighting",      BBQuests::photo_fighting},
        {"photo-flying",        BBQuests::photo_flying},
        {"photo-poison",        BBQuests::photo_poison},
        {"photo-ground",        BBQuests::photo_ground},
        {"photo-rock",          BBQuests::photo_rock},
        {"photo-bug",           BBQuests::photo_bug},
        {"photo-ghost",         BBQuests::photo_ghost},
        {"photo-steel",         BBQuests::photo_steel},
        {"photo-fire",          BBQuests::photo_fire},
        {"photo-water",         BBQuests::photo_water},
        {"photo-grass",         BBQuests::photo_grass},
        {"photo-electric",      BBQuests::photo_electric},
        {"photo-psychic",       BBQuests::photo_psychic},
        {"photo-ice",           BBQuests::photo_ice},
        {"photo-dragon",        BBQuests::photo_dragon},
        {"photo-dark",          BBQuests::photo_dark},
        {"photo-fairy",         BBQuests::photo_fairy},
        {"ditto-central",       BBQuests::ditto_central},
        {"ditto-canyon",        BBQuests::ditto_canyon},
        {"ditto-coastal",       BBQuests::ditto_coastal},
        {"ditto-polar",         BBQuests::ditto_polar},
        {"ditto-savanna",       BBQuests::ditto_savanna},
        {"group-canyon",        BBQuests::group_canyon},
        {"group-coastal",       BBQuests::group_coastal},
        {"group-polar",         BBQuests::group_polar},
        {"group-savanna",       BBQuests::group_savanna},
        {"group-eyewear",       BBQuests::group_eyewear},
        {"group-nonuniform",    BBQuests::group_nonuniform},
        {"group-masks",         BBQuests::group_masks},
        {"sandwich-four",       BBQuests::sandwich_four},
        {"catch-hint",          BBQuests::catch_hint},
        {"catch-hint2",         BBQuests::catch_hint2},
        {"",                    BBQuests::UnableToDetect},
    };
    return data;
}

const std::map<BBQuests, std::string>& BBQuests_ENUM_TO_TOKEN(){
    static std::map<BBQuests, std::string> data = []{
        std::map<BBQuests, std::string> ret;
        for (const auto& [token, quest] : BBQuests_TOKEN_TO_ENUM()){
            ret.emplace(quest, token);
        }
        return ret;
    }();
    return data;
}

}

BBQuests BBQuests_string_to_enum(const std::string& token){
    auto iter = BBQuests_TOKEN_TO_ENUM().find(token);
    if (iter == BBQuests_TOKEN_TO_ENUM().end()){
        return BBQuests::UnableToDetect;
    }
    return iter->second;
}

const std::string& BBQuests_enum_to_string(BBQuests quest){
    static const std::string EMPTY;
    auto iter = BBQuests_ENUM_TO_TOKEN().find(quest);
    if (iter == BBQuests_ENUM_TO_TOKEN().end()){
        return EMPTY;
    }
    return iter->second;
}


}
}
}
