/*  BBQ Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "PokemonSV_BBQOption.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

BBQOption::BBQOption(OCR::LanguageOCROption* language_option)
    : GroupOption("Blueberry Quests", LockMode::UNLOCK_WHILE_RUNNING)
    , m_language_owner(language_option == nullptr
        ? new OCR::LanguageOCROption(
            "<b>Game Language:</b><br>This is required to read quests.",
            IV_READER().languages(),
            LockMode::LOCK_WHILE_RUNNING,
            true
        )
        : nullptr
    )
    , LANGUAGE(language_option == nullptr ? *m_language_owner : *language_option)
    , NUM_QUESTS(
        "<b>Number of Quests to run:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        1000
    )
    , SAVE_NUM_QUESTS(
        "<b>Save and reset the game after attempting this many quests:</b><br>This preserves progress and prevents potential game lags from long runs.<br>0 disables this option.",
        LockMode::UNLOCK_WHILE_RUNNING, 50
    )
    , INVERTED_FLIGHT(
        "<b>Inverted Flight:</b><br>Check this box if inverted flight controls are set.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , QUICKBALL(
        "<b>Catch Quest - Throw Quick Ball:</b><br>When attempting to catch a non-tera Pokemon, use a Quick Ball on the first turn.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , BALL_SELECT(
        "<b>Catch Quest - Ball Select:</b><br>What ball to use when performing a catch quest.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "ultra-ball"
    )
    , NUM_EGGS(
        "<b>Hatch Quest - Number of Eggs:</b><br>Amount of eggs located in your current box.",
        LockMode::UNLOCK_WHILE_RUNNING, 30, 0, 30
    )
    , OUT_OF_EGGS(
        "<b>Hatch Quest - Zero Eggs:</b><br>When out of eggs to hatch, do the selected option.",
        {
            {OOEggs::Stop,      "stop",         "Stop Program"},
            {OOEggs::Reroll,    "reroll",       "Reroll Quest - Costs 50BP"},
            {OOEggs::KeepGoing, "keep-going",   "Keep Going - Warning: Bonus(Red) quests will be blocked!"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        OOEggs::Stop
    )
    , FIX_TIME_FOR_HATCH(
        "<b>Hatch Quest - Fix time for eggs:</b><br>Fix the time before doing a hatch egg quest.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , CATCH_ON_WIN(true)
    , NUM_RETRIES(
        "<b>Number of Retries:</b><br>Reattempt a quest this many times if it fails.",
        LockMode::UNLOCK_WHILE_RUNNING, 1, 0, 5
    )
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time when Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
{
    if (m_language_owner){
        PA_ADD_OPTION(LANGUAGE);
    }
    PA_ADD_OPTION(NUM_QUESTS);
    PA_ADD_OPTION(SAVE_NUM_QUESTS);
    PA_ADD_OPTION(INVERTED_FLIGHT);
    PA_ADD_OPTION(QUICKBALL);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(NUM_EGGS);
    PA_ADD_OPTION(OUT_OF_EGGS);
    PA_ADD_OPTION(FIX_TIME_FOR_HATCH);
    PA_ADD_OPTION(BATTLE_AI);
    PA_ADD_OPTION(CATCH_ON_WIN);
    PA_ADD_OPTION(NUM_RETRIES);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
}


}
}
}
