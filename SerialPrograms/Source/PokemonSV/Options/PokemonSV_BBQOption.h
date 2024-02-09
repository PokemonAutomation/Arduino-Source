/*  BBQ Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BBQOption_H
#define PokemonAutomation_PokemonSV_BBQOption_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSV/Options/PokemonSV_BattleMoveTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class BBQOption : public GroupOption, private ConfigOption::Listener {

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
    OCR::LanguageOCROption& LANGUAGE;

    SimpleIntegerOption<uint64_t> NUM_QUESTS;
    SimpleIntegerOption<uint64_t> SAVE_NUM_QUESTS;

    //For catching pokemon quests
    BooleanCheckBoxOption QUICKBALL;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BattleMoveTable BATTLE_MOVES;

    //For egg hatching quest
    SimpleIntegerOption<uint8_t> NUM_EGGS;

private:
    BBQOption(OCR::LanguageOCROption* language_option);
};

}
}
}
#endif
