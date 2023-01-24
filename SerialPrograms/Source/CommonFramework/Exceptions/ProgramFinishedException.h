/*  Program Finished Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramFinishedException_H
#define PokemonAutomation_ProgramFinishedException_H

#include <memory>
#include "ScreenshotException.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;
class EventNotificationOption;
struct ProgramInfo;
class ProgramEnvironment;
class ConsoleHandle;



//  Thrown when the program requests a normal stop to the program.
//    - This should not be consumed except by the infra.
//    - Non-infra are allowed to catch and rethrow this exception.
class ProgramFinishedException : public ScreenshotException{
public:
    ProgramFinishedException();
    explicit ProgramFinishedException(Logger& logger, std::string message);
    explicit ProgramFinishedException(Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot);
    explicit ProgramFinishedException(ConsoleHandle& console, std::string message, bool take_screenshot);

public:
    virtual const char* name() const override{ return "ProgramFinishedException"; }
    virtual std::string message() const override{ return m_message; }

    virtual void send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const override;
};





}
#endif
