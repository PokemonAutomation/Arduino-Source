/*  Unexpected Battle Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_UnexpectedBattleException_H
#define PokemonAutomation_UnexpectedBattleException_H

#include <memory>
#include "ScreenshotException.h"

namespace PokemonAutomation{

class FatalProgramException;


//  Thrown by subroutines if caught in an wild battle in-game unexpectedly.
//  These include recoverable errors which can be consumed by the program.
class UnexpectedBattleException : public ScreenshotException{
public:
    explicit UnexpectedBattleException(ErrorReport error_report, Logger& logger, std::string message);
    explicit UnexpectedBattleException(ErrorReport error_report, Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot);
    explicit UnexpectedBattleException(ErrorReport error_report, ConsoleHandle& console, std::string message, bool take_screenshot);

    virtual const char* name() const override{ return "UnexpectedBattleException"; }
    virtual std::string message() const override{ return m_message; }

    virtual void send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const override;
};





}
#endif
