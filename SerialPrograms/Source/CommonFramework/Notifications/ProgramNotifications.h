/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramNotifications_H
#define PokemonAutomation_ProgramNotifications_H

#include <vector>
#include <string>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "ProgramInfo.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{

class Logger;
class StatsTracker;
class ProgramEnvironment;



//  Raw Notification
void send_raw_program_notification_with_file(
    Logger& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const std::string& filepath
);
void send_raw_program_notification(
    Logger& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);



//  Program notification with stats.
void send_program_notification_with_file(
    ProgramEnvironment& env, EventNotificationOption& settings,
    Color color,
    const std::string& title,
    std::vector<std::pair<std::string, std::string>> messages,
    const std::string& current_stats_addendum,
    const std::string& filepath
);
void send_program_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    Color color,
    const std::string& title,
    std::vector<std::pair<std::string, std::string>> messages,
    const std::string& current_stats_addendum,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);



//  Other common program notification types.
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
void send_program_fatal_error_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message,
    const ImageViewRGB32& image = ImageViewRGB32(), bool keep_file = false
);






}
#endif
