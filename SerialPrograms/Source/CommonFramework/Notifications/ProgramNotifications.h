/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramNotifications_H
#define PokemonAutomation_ProgramNotifications_H

#include <vector>
#include <string>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "ProgramInfo.h"
#include "MessageAttachment.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{

class StatsTracker;
class ProgramEnvironment;


#if 1
//  This is the generic notification function. You shouldn't need to call this
//  one directly.
void send_program_notification(
    Logger& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);
#endif


void send_program_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    Color color,
    const std::string& title,
    std::vector<std::pair<std::string, std::string>> messages,
    const std::string& current_stats_addendum,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);



void send_program_status_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message = "",
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);

void send_program_finished_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message = "",
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);

void send_program_recoverable_error_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);



//  Notifications without the environment.
void send_program_finished_notification(
    Logger& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const std::string& message,
    std::string current_stats,
    std::string historical_stats,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);
void send_program_fatal_error_notification(
    Logger& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const std::string& message,
    std::string current_stats,
    std::string historical_stats,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);






void send_program_telemetry(
    Logger& logger, bool is_error, Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const std::string& file
);


}
#endif
