/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "OperationFailedException.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


OperationFailedException::OperationFailedException(
    ErrorReport error_report_mode,
    std::string message
)
    : m_error_report_mode(error_report_mode)
    , m_message(std::move(message))
{}


void OperationFailedException::send_fatal_error_notif_and_telemetry_report(
    ProgramEnvironment& env,
    EventNotificationOption& notif_settings
){
    send_program_fatal_error_notification_and_telemetry_report(
        env, &env.logger(), env.program_info(),
        notif_settings,
        error_report_mode(),
        message(),
        name()
    );
}


void OperationFailedException::send_recoverable_error_notif_and_telemetry_report(
    ProgramEnvironment& env,
    EventNotificationOption& notif_settings
){
    send_program_recoverable_error_notification_and_telemetry_report(
        env, &env.logger(), env.program_info(), 
        notif_settings, 
        error_report_mode(),
        message(),
        name()
    );
}



}
