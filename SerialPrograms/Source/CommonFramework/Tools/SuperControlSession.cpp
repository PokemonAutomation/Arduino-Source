/*  Super-Control Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "ConsoleHandle.h"
#include "InterruptableCommands.h"
#include "SuperControlSession.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


SuperControlSession::~SuperControlSession(){}

SuperControlSession::SuperControlSession(
    ProgramEnvironment& env, ConsoleHandle& console,
        std::chrono::milliseconds state_period,
        std::chrono::milliseconds visual_period,
        std::chrono::milliseconds audio_period
)
    : m_env(env)
    , m_console(console)
    , m_state_period(state_period)
    , m_visual_period(visual_period)
    , m_audio_period(audio_period)
{}

void SuperControlSession::operator+=(VisualInferenceCallback& callback){
    m_visual_callbacks.emplace_back(&callback);
}
void SuperControlSession::operator+=(AudioInferenceCallback& callback){
    m_audio_callbacks.emplace_back(&callback);
}
void SuperControlSession::register_state_command(size_t state, std::function<bool()>&& action){
    auto iter = m_state_actions.find(state);
    if (iter != m_state_actions.end()){
        PA_THROW_StringException("Duplicate State Enum: " + std::to_string(state));
    }
    m_state_actions.emplace(state, std::move(action));
}
bool SuperControlSession::run_state_action(size_t state){
    auto iter = m_state_actions.find(state);
    if (iter == m_state_actions.end()){
        PA_THROW_StringException("Unknown State Enum: " + std::to_string(state));
    }

    //  Session isn't even active.
    if (!m_active_command){
        return false;
    }

    //  If we're already in the state and a command is running, don't overwrite it.
    if (state == m_state && m_active_command->command_is_running()){
        return false;
    }

    //  Run the state.
    m_state = state;
    return iter->second();
}

void SuperControlSession::run_session(){

    std::unique_ptr<AsyncVisualInferenceSession> visual;
    if (!m_visual_callbacks.empty()){
        visual.reset(new AsyncVisualInferenceSession(m_env, m_console, m_console, m_console, m_visual_period));
        for (VisualInferenceCallback* callback : m_visual_callbacks){
            *visual += *callback;
        }
    }

#if 0
    std::unique_ptr<AsyncAudioInferenceSession> audio;
    if (!m_audio_callbacks.empty()){
        audio.reset(new AsyncAudioInferenceSession(m_env, m_console, m_console, m_console, m_audio_period));
        for (AudioInferenceCallback* callback : m_audio_callbacks){
            *audio += *callback;
        }
    }
#endif

    m_active_command.reset(new AsyncCommandSession(m_env, m_console.botbase()));

    WallClock now = std::chrono::system_clock::now();
    WallClock next_tick = now + m_state_period;

    m_state = 0;

    while (true){
        m_env.check_stopping();
        if (run_state(*m_active_command, std::chrono::system_clock::now())){
            break;
        }

        now = std::chrono::system_clock::now();
        auto wait = next_tick - now;
        if (wait <= std::chrono::milliseconds(0)){
            next_tick = now + m_state_period;
        }else{
            std::mutex lock;
            std::condition_variable cv;
            ProgramStopNotificationScope scope(m_env, lock, cv);
            std::unique_lock<std::mutex> lg(lock);
            cv.wait_until(
                lg, next_tick,
                [=]{ return m_env.is_stopping(); }
            );
            next_tick += m_state_period;
        }
    }


//    cout << "SuperControlSession::run_session() - stop" << endl;
    m_active_command->stop_session();

#if 0
    if (audio){
        audio->stop();
    }
#endif

    if (visual){
        visual->stop();
    }
//    cout << "SuperControlSession::run_session() - end" << endl;
}



}
