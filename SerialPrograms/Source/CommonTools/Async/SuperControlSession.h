/*  Super-Control Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_SuperControlSession_H
#define PokemonAutomation_CommonTools_SuperControlSession_H

#include <memory>
#include <vector>
#include <map>
#include <chrono>
#include <functional>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{

class ProgramEnvironment;
class VideoStream;
template <typename ControllerType> class AsyncCommandSession;
class AudioInferenceCallback;
class VisualInferenceCallback;



//
//  Although this class asynchronously runs inference and commands, none of the
//  public methods are actually thread-safe with each other.
//
//  You must fully construct the class (register all the callbacks and actions)
//  prior to calling "run_session()".
//
//  "run_session()" will run the state machine which will periodically call
//  "run_state()" at the user-specified period.
//
//  Inside the child class' "run_state()" function, it will call "run_state_action()"
//  with a registered enum/action.
//

template <typename ControllerType>
class SuperControlSession{
    using ControllerContextType = typename ControllerType::ContextType;

public:
    ~SuperControlSession();
    void run_session();

protected:
    //  Construction
    SuperControlSession(
        ProgramEnvironment& env, VideoStream& stream, ControllerContextType& context,
        std::chrono::milliseconds state_period = std::chrono::milliseconds(100),
        std::chrono::milliseconds visual_period = std::chrono::milliseconds(50),
        std::chrono::milliseconds audio_period = std::chrono::milliseconds(20)
    );

    void operator+=(VisualInferenceCallback& callback);
    void operator+=(AudioInferenceCallback& callback);

    void register_state_command(size_t state, std::function<bool()>&& action);


protected:
    WallClock start_time() const{ return m_start_time; }

    size_t last_state() const{ return m_last_state; }
    WallClock last_state_change() const{ return m_last_state_change; }

    //  Return true if we should stop.
    virtual bool run_state(AsyncCommandSession<ControllerType>& commands, WallClock timestamp) = 0;

    //  Run the specified state. This is debounced such that the registered
    //  command will only run if we're not already in the state and the command
    //  session is still running.
    bool run_state_action(size_t state);


protected:
    ProgramEnvironment& m_env;
    VideoStream& m_stream;
    ControllerContextType& m_context;
    std::unique_ptr<AsyncCommandSession<ControllerType>> m_active_command;

private:
    const std::chrono::milliseconds m_state_period;
    const std::chrono::milliseconds m_visual_period;
    const std::chrono::milliseconds m_audio_period;

    std::vector<VisualInferenceCallback*> m_visual_callbacks;
    std::vector<AudioInferenceCallback*> m_audio_callbacks;

    //  These are actions
    std::map<size_t, std::function<bool()>> m_state_actions;

private:
    //  Run-time state.
    WallClock m_start_time;
    size_t m_last_state;
    WallClock m_last_state_change;
};



}
#endif
