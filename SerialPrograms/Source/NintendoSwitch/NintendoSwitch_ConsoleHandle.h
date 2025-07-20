/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ConsoleHandle_H
#define PokemonAutomation_NintendoSwitch_ConsoleHandle_H

#include <memory>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch_ConsoleState.h"

namespace PokemonAutomation{
    class ThreadHandle;
    class ThreadUtilizationStat;
    class ThreadPoolUtilizationStat;
namespace NintendoSwitch{

class ConsoleHandle : public VideoStream{
public:
    ~ConsoleHandle();
//    ConsoleHandle(ConsoleHandle&& x);
//    void operator=(ConsoleHandle&& x) = delete;
    ConsoleHandle(const ConsoleHandle& x) = delete;
    void operator=(const ConsoleHandle& x) = delete;


public:
    ConsoleHandle(
        size_t index,
        Logger& logger,
        AbstractController& controller,
        VideoFeed& video,
        VideoOverlay& overlay,
        AudioFeed& audio,
        const StreamHistorySession& history
    );

    size_t index() const{ return m_index; }

    template <typename ControllerType = AbstractController>
    ControllerType& controller(){
        ControllerType* ret = dynamic_cast<ControllerType*>(&m_controller);
        if (ret){
            return *ret;
        }
        throw InternalProgramError(&logger(), PA_CURRENT_FUNCTION, "Unable to cast controller.");
    }

    //  REMOVE: Temporary for refactor.
    ProController& pro_controller(){
        ProController* ret = dynamic_cast<ProController*>(&m_controller);
        if (ret){
            return *ret;
        }
        throw InternalProgramError(&logger(), PA_CURRENT_FUNCTION, "Unable to cast to ProController.");
    }


    operator Logger&(){ return logger(); }
    operator VideoFeed&(){ return video(); }
    operator VideoOverlay&(){ return overlay(); }
    operator AudioFeed&() { return audio(); }
    operator const StreamHistorySession&() const{ return history(); }

    ConsoleState& state(){ return m_console_state; }
    operator ConsoleState&(){ return m_console_state; }


private:
    size_t m_index;
    AbstractController& m_controller;

    ConsoleState m_console_state;

    std::unique_ptr<ThreadPoolUtilizationStat> m_realtime_inference_utilization;
    std::unique_ptr<ThreadPoolUtilizationStat> m_normal_inference_utilization;
    std::unique_ptr<ThreadUtilizationStat> m_thread_utilization;
};




}
}
#endif


