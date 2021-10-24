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
#include <QString>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "Pokemon_EncounterStats.h"
#include "Pokemon_DataTypes.h"

namespace PokemonAutomation{
namespace Pokemon{


void send_encounter_notification(
    Logger& logger,
    EventNotificationOption& settings_nonshiny,
    EventNotificationOption& settings_shiny,
    const QString& program,
    const std::set<std::string>* slugs,
    const ShinyDetectionResult& result,
    const StatsTracker* session_stats = nullptr,
    const EncounterFrequencies* frequencies = nullptr,
    const StatsTracker* alltime_stats = nullptr
);

void send_catch_notification(
    Logger& logger,
    EventNotificationOption& settings_catch_success,
    EventNotificationOption& settings_catch_failed,
    const QString& program,
    const std::set<std::string>* pokemon_slugs,
    const std::string& ball_slug, int balls_used,
    bool success
);



}
}
#endif
