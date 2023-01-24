/*  Fatal Program Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FatalProgramException_H
#define PokemonAutomation_FatalProgramException_H

#include <memory>
#include "ScreenshotException.h"

namespace PokemonAutomation{


//  A generic exception that should not be caught outside of infra.
class FatalProgramException : public ScreenshotException{
public:
    FatalProgramException(ScreenshotException&& e);
    explicit FatalProgramException(Logger& logger, std::string message);
    explicit FatalProgramException(Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot);
    explicit FatalProgramException(ConsoleHandle& console, std::string message, bool take_screenshot);

    virtual const char* name() const override{ return "FatalProgramException"; }
    virtual std::string message() const override{ return m_message; }

    virtual void send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const override;
};





}
#endif
