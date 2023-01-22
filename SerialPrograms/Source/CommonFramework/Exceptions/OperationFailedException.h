/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OperationFailedException_H
#define PokemonAutomation_OperationFailedException_H

#include <memory>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;
class EventNotificationOption;
struct ProgramInfo;
class ProgramEnvironment;
class ConsoleHandle;


//  Thrown by subroutines if they fail for an in-game reason.
//  These include recoverable errors which can be consumed by the program.
class OperationFailedException : public Exception{
public:
    ~OperationFailedException();
    OperationFailedException(const OperationFailedException&) = delete;
    OperationFailedException& operator=(const OperationFailedException&) = delete;
    OperationFailedException(OperationFailedException&&);
    OperationFailedException& operator=(OperationFailedException&&);

public:
    explicit OperationFailedException(Logger& logger, std::string message);
    explicit OperationFailedException(Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot);
    explicit OperationFailedException(ConsoleHandle& console, std::string message, bool take_screenshot);

    virtual const char* name() const override{ return "OperationFailedException"; }
    virtual std::string message() const override{ return m_message; }
    ImageViewRGB32 screenshot() const;

    void send_notification(ProgramEnvironment& env,EventNotificationOption& notification) const;

private:
    std::string m_message;
    std::shared_ptr<const ImageRGB32> m_screenshot;
};





}
#endif
