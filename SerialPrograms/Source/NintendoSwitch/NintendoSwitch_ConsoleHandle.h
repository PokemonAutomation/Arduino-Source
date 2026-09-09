/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ConsoleHandle_H
#define PokemonAutomation_NintendoSwitch_ConsoleHandle_H

#include <memory>
#include "CommonFramework/Tools/VideoStream.h"
#include "Controllers/Controller.h"
#include "Controllers/NullController.h"
#include "NintendoSwitch_ConsoleState.h"

namespace PokemonAutomation{
    class ThreadHandle;
    class ThreadUtilizationStat;
    class ThreadPoolUtilizationStat;
namespace ConsoleInfra{
    class ConsoleSystemSession;
}
namespace NintendoSwitch{

class ConsoleHandle : public VideoStream{
public:
    ~ConsoleHandle();
//    ConsoleHandle(ConsoleHandle&& x);
//    void operator=(ConsoleHandle&& x) = delete;
    ConsoleHandle(const ConsoleHandle& x) = delete;
    void operator=(const ConsoleHandle& x) = delete;


public:
    ConsoleHandle(ConsoleInfra::ConsoleSystemSession& session);

    size_t index() const{ return m_index; }

    operator Logger&(){ return logger(); }
    operator VideoFeed&(){ return video(); }
    operator VideoOverlay&(){ return overlay(); }
    operator AudioFeed&(){ return audio(); }
    operator const StreamHistorySession&() const{ return history(); }


public:
    size_t controllers() const;
    AbstractController& controller(size_t index);

    template <typename ControllerType = AbstractController>
    ControllerType& controller(){
        return controller<ControllerType>(0);
    }

    template <typename ControllerType = AbstractController>
    ControllerType& controller(size_t index){
        return controller(index).cast_with_exception<ControllerType>();
    }


public:
    ConsoleState& state(){ return m_console_state; }
    operator ConsoleState&(){ return m_console_state; }


public:
    void wait_for_all_controllers() noexcept;
    void cancel_all_controllers() noexcept;


private:
    ConsoleInfra::ConsoleSystemSession& m_session;
    size_t m_index;
    NullController m_null_controller;

    ConsoleState m_console_state;

    std::unique_ptr<ThreadPoolUtilizationStat> m_realtime_inference_utilization;
    std::unique_ptr<ThreadPoolUtilizationStat> m_normal_inference_utilization;
    std::unique_ptr<ThreadUtilizationStat> m_thread_utilization;
};




}
}
#endif


