/*  Encounter Bot Common
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterBotCommon_H
#define PokemonAutomation_PokemonSwSh_EncounterBotCommon_H

#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



struct EncounterBotCommonSettings{
    const EncounterFilterOption& filter;
    bool video_on_shiny;
    EncounterBotNotificationLevel notification_level;
    ScreenshotMode shiny_screenshot;
};


class EncounterBotCommonOptions : public BatchOption{
public:
    EncounterBotCommonOptions(bool rare_stars, bool enable_overrides)
        : FILTER(rare_stars, enable_overrides)
        , VIDEO_ON_SHINY(
            "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
            true
        )
    {
        PA_ADD_OPTION(FILTER);
        PA_ADD_OPTION(VIDEO_ON_SHINY);
        PA_ADD_OPTION(NOTIFICATION_LEVEL);
        PA_ADD_OPTION(SHINY_SCREENSHOT);
    }

    operator EncounterBotCommonSettings() const{
        return {FILTER, VIDEO_ON_SHINY, NOTIFICATION_LEVEL, SHINY_SCREENSHOT};
    }

    EncounterFilterOption FILTER;
    BooleanCheckBoxOption VIDEO_ON_SHINY;
    EncounterBotNotifications NOTIFICATION_LEVEL;
    EncounterBotScreenshotOption SHINY_SCREENSHOT;
};




}
}
}
#endif
