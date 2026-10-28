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
#include "VideoGames/PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_TeraAIOption.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_TeraCatchOnWinOption.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_BBQuests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

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
