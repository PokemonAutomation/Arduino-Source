/*  Check for Updates Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_CheckForUpdatesOption_H
#define PokemonAutomation_Options_CheckForUpdatesOption_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Globals.h"

namespace PokemonAutomation{


class CheckForUpdatesOption : public GroupOption{
public:
    CheckForUpdatesOption()
        : GroupOption(
            "Check for Updates",
            LockMode::UNLOCK_WHILE_RUNNING,
            EnableMode::ALWAYS_ENABLED,
            true
        )
        , RELEASE0(
            "<b>New Releases:</b><br>Automatically check for new stable releases.",
            LockMode::UNLOCK_WHILE_RUNNING,
            true
        )
        , PUBLIC_BETA0(
            "<b>Public Betas:</b><br>Automatically check for new public betas.",
            LockMode::UNLOCK_WHILE_RUNNING,
            IS_BETA_VERSION
        )
        , PRIVATE_BETA0(
            "<b>Private Betas:</b><br>Automatically check for new private betas.",
            LockMode::UNLOCK_WHILE_RUNNING,
            IS_BETA_VERSION
        )
        ,SKIP_VERSION(
            false,
            "<b>Skip this Version:</b><br>Skip this version and don't notify until the next update.",
            LockMode::UNLOCK_WHILE_RUNNING,
            "",
            "0.50.0"
        )
    {
        PA_ADD_OPTION(RELEASE0);
        PA_ADD_OPTION(PUBLIC_BETA0);
        if (IS_BETA_VERSION){
            PA_ADD_OPTION(PRIVATE_BETA0);
        }
        PA_ADD_OPTION(SKIP_VERSION);
    }

public:
    BooleanCheckBoxOption RELEASE0;
    BooleanCheckBoxOption PUBLIC_BETA0;
    BooleanCheckBoxOption PRIVATE_BETA0;
    StringOption SKIP_VERSION;
};


}
#endif
