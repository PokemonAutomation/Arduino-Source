/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OperationFailedException_H
#define PokemonAutomation_OperationFailedException_H

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"

namespace PokemonAutomation{


//  Thrown by subroutines if they fail for an in-game reason.
//  These include recoverable errors which can be consumed by the program.
class OperationFailedException : public Exception{
public:
    OperationFailedException(
        ErrorReport error_report_mode,
        std::string message
    );

    ErrorReport error_report_mode() const { return m_error_report_mode; };

    virtual const char* name() const override{ return "OperationFailedException"; }
    virtual std::string message() const override{ return m_message; }

    virtual void send_recoverable_error_notif_and_telemetry_report(
        ProgramEnvironment& env,
        EventNotificationOption& notif_settings
    );

    virtual void send_fatal_error_notif_and_telemetry_report(
        ProgramEnvironment& env,
        EventNotificationOption& notif_settings
    );


private:
    ErrorReport m_error_report_mode;
    std::string m_message;
};





}
#endif
