/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_ConsoleHandle_H
#define PokemonAutomation_GameConsole_ConsoleHandle_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "Controllers/Controller.h"

namespace PokemonAutomation{
namespace GameConsole{

class ConsoleSystemSession;



class ConsoleHandle : public VideoStream{
public:
    virtual ~ConsoleHandle();
    ConsoleHandle(ConsoleHandle&& x) = delete;
    void operator=(ConsoleHandle&& x) = delete;
    ConsoleHandle(const ConsoleHandle& x) = delete;
    void operator=(const ConsoleHandle& x) = delete;


public:
    ConsoleHandle(ConsoleSystemSession& session);

    size_t index() const;

    //  The console session this handle belongs to. Programs use it to listen for
    //  session events, e.g. the user's keyboard input (see ConsoleSystemSession::Listener).
    ConsoleSystemSession& system_session();

    operator Logger&(){ return logger(); }
    operator VideoFeed&(){ return video(); }
    operator VideoOverlay&(){ return overlay(); }
    operator AudioFeed&(){ return audio(); }
    operator const StreamHistorySession&() const{ return history(); }


public:
    size_t controllers() const;
    AbstractController& controller(size_t index);

    template <typename ControllerType = AbstractController>
    ControllerType& controller(size_t index = 0){
        return controller(index).cast_with_exception<ControllerType>();
    }


public:
    void wait_for_all_controllers() noexcept;
    void cancel_all_controllers() noexcept;


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
}
#endif


