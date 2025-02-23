/*  Check for Updates Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_CheckForUpdatesOption_H
#define PokemonAutomation_Options_CheckForUpdatesOption_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Globals.h"

namespace PokemonAutomation{


class CheckForUpdatesOption : public GroupOption{
public:
    CheckForUpdatesOption()
        : GroupOption(
            "Check for Updates",
            LockMode::UNLOCK_WHILE_RUNNING,
            EnableMode::ALWAYS_ENABLED,
            false
        )
        , RELEASE(
            "<b>New Releases:</b><br>Automatically check for new stable releases.",
            LockMode::UNLOCK_WHILE_RUNNING,
            true
        )
        , BETA(
            "<b>Public Betas:</b><br>Automatically check for new public betas.",
            LockMode::UNLOCK_WHILE_RUNNING,
            IS_BETA_VERSION
        )
        , PRIVATE_BETA(
            "<b>Private Betas:</b><br>Automatically check for new private betas.",
            LockMode::UNLOCK_WHILE_RUNNING,
            IS_BETA_VERSION
        )
    {
        PA_ADD_OPTION(RELEASE);
        PA_ADD_OPTION(BETA);
        if (IS_BETA_VERSION){
            PA_ADD_OPTION(PRIVATE_BETA);
        }
    }

public:
    BooleanCheckBoxOption RELEASE;
    BooleanCheckBoxOption BETA;
    BooleanCheckBoxOption PRIVATE_BETA;
};


}
#endif
