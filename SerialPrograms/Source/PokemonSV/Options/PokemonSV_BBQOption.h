/*  BBQ Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BBQOption_H
#define PokemonAutomation_PokemonSV_BBQOption_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"
#include "PokemonSV/Options/PokemonSV_TeraCatchOnWinOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

enum class BBQuests{
    auto_10, make_tm, pickup_10, sneak_up, photo_fly, photo_swim, photo_canyon, photo_coastal, photo_polar, photo_savanna, tera_self_defeat,
    travel_500, catch_any, catch_normal, catch_fighting, catch_flying, catch_poison, catch_ground, catch_rock, catch_bug, catch_ghost, catch_steel,
    catch_fire, catch_water, catch_grass, catch_electric, catch_psychic, catch_ice, catch_dragon, catch_dark, catch_fairy,
    wash_pokemon, wild_tera, auto_30, tera_raid, sandwich_three, bitter_sandwich, sweet_sandwich, salty_sandwich, sour_sandwich, spicy_sandwich, hatch_egg,
    photo_normal, photo_fighting, photo_flying, photo_poison, photo_ground, photo_rock, photo_bug, photo_ghost, photo_steel, photo_fire, photo_water,
    photo_grass, photo_electric, photo_psychic, photo_ice, photo_dragon, photo_dark, photo_fairy,
    ditto_central, ditto_canyon, ditto_coastal, ditto_polar, ditto_savanna, group_canyon, group_coastal, group_polar, group_savanna, group_eyewear, group_nonuniform,
    group_masks, sandwich_four, catch_hint, catch_hint2,
    UnableToDetect
};

enum class BBQAction{
    run,
    skip,
    reroll
};

//Quest exclusion table
const EnumDropdownDatabase<BBQuests>& BBQuests_database();

class BBQuestTableRow : public EditableTableRow{
public:
    BBQuestTableRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    EnumDropdownCell<BBQuests> quest;
    EnumDropdownCell<BBQAction> action;
};

class BBQuestTable : public EditableTableOption_t<BBQuestTableRow>{
public:
    BBQuestTable();

    virtual std::vector<std::string> make_header() const;
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};


//BBQ Options
class BBQOption : public GroupOption{

public:
    //  Include the language option in the box.
    BBQOption()
        : BBQOption(nullptr)
    {}

    //  Don't include language option. Give it one instead.
    BBQOption(OCR::LanguageOCROption& language_option)
        : BBQOption(&language_option)
    {}

private:
    std::unique_ptr<OCR::LanguageOCROption> m_language_owner;

public:
    enum class OOEggs {
        Stop,
        Reroll,
        KeepGoing,
    };

    OCR::LanguageOCROption& LANGUAGE;

    SimpleIntegerOption<uint64_t> NUM_QUESTS;
    SimpleIntegerOption<uint64_t> SAVE_NUM_QUESTS;

    BBQuestTable QUEST_EXCLUSIONS;

    BooleanCheckBoxOption INVERTED_FLIGHT;

    //For catching pokemon quests
    BooleanCheckBoxOption QUICKBALL;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    //BattleMoveTable BATTLE_MOVES; No need, not catching legendaries.

    //For egg hatching quest
    SimpleIntegerOption<uint8_t> NUM_EGGS;
    EnumDropdownOption<OOEggs> OUT_OF_EGGS;
    BooleanCheckBoxOption FIX_TIME_FOR_HATCH;

    //Tera raid
    TeraAIOption BATTLE_AI;
    TeraFarmerCatchOnWin CATCH_ON_WIN;

    SimpleIntegerOption<uint8_t> NUM_RETRIES;

    BooleanCheckBoxOption FIX_TIME_WHEN_DONE;

private:
    BBQOption(OCR::LanguageOCROption* language_option);
};

}
}
}
#endif
