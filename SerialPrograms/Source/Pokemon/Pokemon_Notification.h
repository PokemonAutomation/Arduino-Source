/*  Shiny Notification
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_ShinyNotification_H
#define PokemonAutomation_Pokemon_ShinyNotification_H

#include <string>
#include <set>
#include <chrono>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "Pokemon_EncounterStats.h"
#include "Pokemon_DataTypes.h"

namespace PokemonAutomation{
namespace Pokemon{


const Color COLOR_STAR_SHINY(0xffff99);
const Color COLOR_SQUARE_SHINY(0xb266ff);



struct EncounterResult{
    std::set<std::string> slug_candidates;
    ShinyType shininess = ShinyType::UNKNOWN;
};


void send_encounter_notification(
    ProgramEnvironment& env,
    EventNotificationOption& settings_nonshiny,
    EventNotificationOption& settings_shiny,
    bool enable_names, bool shiny_detected,
    const std::vector<EncounterResult>& results,
    double alpha,   //  Set to std::nan("") to hide the field.
    const ImageViewRGB32& screenshot = ImageViewRGB32(),
    const EncounterFrequencies* frequencies = nullptr
);

void send_catch_notification(
    LoggerQt& logger,
    EventNotificationOption& settings_catch_success,
    EventNotificationOption& settings_catch_failed,
    const ProgramInfo& info,
    const std::set<std::string>* pokemon_slugs,
    const std::string& ball_slug, int balls_used,
    bool success
);



}
}
#endif
