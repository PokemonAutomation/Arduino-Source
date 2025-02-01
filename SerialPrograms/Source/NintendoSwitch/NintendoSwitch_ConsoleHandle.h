/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ConsoleHandle_H
#define PokemonAutomation_NintendoSwitch_ConsoleHandle_H

#include <memory>
#include "CommonFramework/Tools/VideoStream.h"
#include "Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    class ThreadHandle;
    class ThreadUtilizationStat;
namespace NintendoSwitch{

class ConsoleHandle : public VideoStream{
public:
    ConsoleHandle(ConsoleHandle&& x);
    void operator=(ConsoleHandle&& x) = delete;
    ConsoleHandle(const ConsoleHandle& x) = delete;
    void operator=(const ConsoleHandle& x) = delete;
    ~ConsoleHandle();

public:
    ConsoleHandle(
        size_t index,
        Logger& logger,
        SwitchController& controller,
        VideoFeed& video,
        VideoOverlay& overlay,
        AudioFeed& audio,
        const StreamHistorySession& history
    );

    size_t index() const{ return m_index; }

    SwitchController& controller(){ return m_controller; }


    operator Logger&(){ return logger(); }
    operator VideoFeed&(){ return video(); }
    operator VideoOverlay&(){ return overlay(); }
    operator AudioFeed&() { return audio(); }
    operator const StreamHistorySession&() const{ return history(); }


private:
    size_t m_index;
    SwitchController& m_controller;

    std::unique_ptr<ThreadUtilizationStat> m_thread_utilization;
};




}
}
#endif


