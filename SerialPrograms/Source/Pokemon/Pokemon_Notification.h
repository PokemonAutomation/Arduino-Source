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
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "Pokemon_EncounterStats.h"
#include "Pokemon_Types.h"

namespace PokemonAutomation{
namespace Pokemon{


void send_encounter_notification(
    Logger* logger,
    const QString& program,
    const std::set<std::string>* slugs,
    ShinyType shiny,
    const StatsTracker* session_stats = nullptr,
    const EncounterFrequencies* frequencies = nullptr,
    const StatsTracker* alltime_stats = nullptr
);


class EncounterNotificationSender{
public:
    EncounterNotificationSender(
        EncounterBotNotificationLevel notification_level,
        std::chrono::seconds period = std::chrono::seconds(3600)
    );

    void send_notification(
        Logger* logger,
        const QString& program,
        const std::set<std::string>* slugs,
        ShinyType shiny,
        const StatsTracker* session_stats = nullptr,
        const EncounterFrequencies* frequencies = nullptr,
        const StatsTracker* alltime_stats = nullptr
    );

private:
    EncounterBotNotificationLevel m_notification_level;
    std::chrono::seconds m_notification_period;
    std::chrono::system_clock::time_point m_last_notification;
};



}
}
#endif
