/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "GameConsole/ConsoleSystemSession.h"
#include "NintendoSwitch_ConsoleHandle.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


//ConsoleHandle::ConsoleHandle(ConsoleHandle&& x) = default;
ConsoleHandle::~ConsoleHandle(){
    overlay().remove_stat(*m_thread_utilization);
    overlay().remove_stat(*m_normal_inference_utilization);
    overlay().remove_stat(*m_realtime_inference_utilization);
}


ConsoleHandle::ConsoleHandle(GameConsole::ConsoleSystemSession& session)
    : VideoStream(
        session.logger(),
        session.audio(),
        session.video(),
        session.stream_history(),
        session.overlay()
    )
    , m_session(session)
    , m_index(session.console_number())
    , m_null_controller(session.logger())
    , m_realtime_inference_utilization(
        new ThreadPoolUtilizationStat(
            GlobalThreadPools::computation_realtime(),
            "Real-Time Pool"
        )
    )
    , m_normal_inference_utilization(
        new ThreadPoolUtilizationStat(
            GlobalThreadPools::computation_normal(),
            "Normal Pool"
        )
    )
    , m_thread_utilization(
        new ThreadUtilizationStat(
            current_thread_handle(),
            "Program Thread:"
        )
    )
{
    session.overlay().add_stat(*m_realtime_inference_utilization);
    session.overlay().add_stat(*m_normal_inference_utilization);
    session.overlay().add_stat(*m_thread_utilization);
}


size_t ConsoleHandle::controllers() const{
    return m_session.controllers();
}

AbstractController& ConsoleHandle::controller(size_t index){
    if (index >= m_session.controllers()){
        return m_null_controller;
    }
    AbstractController* ptr = m_session.controller(index).controller();
    if (ptr == nullptr){
        return m_null_controller;
    }
    return *ptr;
}

void ConsoleHandle::wait_for_all_controllers() noexcept{
    size_t total = m_session.controllers();
    for (size_t c = 0; c < total; c++){
        m_session.controller(c).try_run<AbstractController>([](AbstractController& controller){
            controller.wait_for_all(nullptr);
        });
    }
}
void ConsoleHandle::cancel_all_controllers() noexcept{
    size_t total = m_session.controllers();
    for (size_t c = 0; c < total; c++){
        m_session.controller(c).try_run<AbstractController>([](AbstractController& controller){
            controller.cancel_all_commands();
        });
    }
}




}
}
