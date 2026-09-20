/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "Controllers/NullController.h"
#include "Framework/ConsoleSystemSession.h"
#include "ConsoleHandle.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace GameConsole{



struct ConsoleHandle::Data{
    ConsoleSystemSession& m_session;
    size_t m_index;
    NullController m_null_controller;

    ThreadPoolUtilizationStat m_realtime_inference_utilization;
    ThreadPoolUtilizationStat m_normal_inference_utilization;
    ThreadUtilizationStat m_thread_utilization;

    Data(ConsoleSystemSession& session)
        : m_session(session)
        , m_index(session.console_number())
        , m_null_controller(session.logger())
        , m_realtime_inference_utilization(
            GlobalThreadPools::computation_realtime(),
            "Real-Time Pool"
        )
        , m_normal_inference_utilization(
            GlobalThreadPools::computation_normal(),
            "Normal Pool"
        )
        , m_thread_utilization(
            current_thread_handle(),
            "Program Thread:"
        )
    {}
};






//ConsoleHandle::ConsoleHandle(ConsoleHandle&& x) = default;
ConsoleHandle::~ConsoleHandle(){
    overlay().remove_stat(m_data->m_thread_utilization);
    overlay().remove_stat(m_data->m_normal_inference_utilization);
    overlay().remove_stat(m_data->m_realtime_inference_utilization);
}


ConsoleHandle::ConsoleHandle(ConsoleSystemSession& session)
    : VideoStream(
        session.logger(),
        session.audio(),
        session.video(),
        session.stream_history(),
        session.overlay()
    )
    , m_data(CONSTRUCT_TOKEN, session)
{
    session.overlay().add_stat(m_data->m_realtime_inference_utilization);
    session.overlay().add_stat(m_data->m_normal_inference_utilization);
    session.overlay().add_stat(m_data->m_thread_utilization);
}



size_t ConsoleHandle::index() const{
    return m_data->m_index;
}
size_t ConsoleHandle::controllers() const{
    return m_data->m_session.controllers();
}
AbstractController& ConsoleHandle::controller(size_t index){
    if (index >= m_data->m_session.controllers()){
        throw InternalProgramError(
            &logger(),
            PA_CURRENT_FUNCTION,
            "Controller index is out-of-bounds: " + std::to_string(index)
        );
    }
    AbstractController* ptr = m_data->m_session.controller(index).controller();
    if (ptr == nullptr){
        return m_data->m_null_controller;
    }
    return *ptr;
}

void ConsoleHandle::wait_for_all_controllers() noexcept{
    size_t total = m_data->m_session.controllers();
    for (size_t c = 0; c < total; c++){
        m_data->m_session.controller(c).try_run<AbstractController>([](AbstractController& controller){
            controller.wait_for_all(nullptr);
        });
    }
}
void ConsoleHandle::cancel_all_controllers() noexcept{
    size_t total = m_data->m_session.controllers();
    for (size_t c = 0; c < total; c++){
        m_data->m_session.controller(c).try_run<AbstractController>([](AbstractController& controller){
            controller.cancel_all_commands();
        });
    }
}




}
}
