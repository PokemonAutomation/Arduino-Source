/*  Super-Control Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/InferenceInfra/AudioInferenceCallback.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "ProgramEnvironment.h"
#include "InterruptableCommands.h"
#include "SuperControlSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


SuperControlSession::~SuperControlSession(){}

SuperControlSession::SuperControlSession(
    ProgramEnvironment& env, VideoStream& stream, BotBaseContext& context,
        std::chrono::milliseconds state_period,
        std::chrono::milliseconds visual_period,
        std::chrono::milliseconds audio_period
)
    : m_env(env)
    , m_stream(stream)
    , m_context(context)
    , m_state_period(state_period)
    , m_visual_period(visual_period)
    , m_audio_period(audio_period)
    , m_last_state(0)
    , m_last_state_change(current_time())
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
        throw InternalProgramError(&m_stream.logger(), PA_CURRENT_FUNCTION, "Duplicate State Enum: " + std::to_string(state));
    }
    m_state_actions.emplace(state, std::move(action));
}
bool SuperControlSession::run_state_action(size_t state){
    auto iter = m_state_actions.find(state);
    if (iter == m_state_actions.end()){
        throw InternalProgramError(&m_stream.logger(), PA_CURRENT_FUNCTION, "Unknown State Enum: " + std::to_string(state));
    }

    //  Session isn't even active.
    if (!m_active_command){
        return false;
    }

    //  If we're already in the state and a command is running, don't overwrite it.
    if (state == m_last_state && m_active_command->command_is_running()){
        return false;
    }

    //  Run the state.
    m_last_state = state;
    m_last_state_change = current_time();
    return iter->second();
}

void SuperControlSession::run_session(){
    m_start_time = current_time();

    m_active_command.reset(
        new AsyncCommandSession(
            m_context, m_env.logger(), m_env.realtime_dispatcher(),
            m_context.botbase()
        )
    );

    std::vector<PeriodicInferenceCallback> callbacks;
    for (VisualInferenceCallback* callback : m_visual_callbacks){
        callbacks.emplace_back(PeriodicInferenceCallback{*callback, m_visual_period});
    }
    for (AudioInferenceCallback* callback : m_audio_callbacks){
        callbacks.emplace_back(PeriodicInferenceCallback{*callback, m_audio_period});
    }
    InferenceSession session(
        m_context, m_stream,
        callbacks,
        m_visual_period
    );

    WallClock now = current_time();
    WallClock next_tick = now + m_state_period;

    m_last_state = 0;

    while (true){
        //  Check stop conditions.
        m_context.throw_if_cancelled();

        if (run_state(*m_active_command, current_time())){
            break;
        }

        now = current_time();
        if (now >= next_tick){
            next_tick = now + m_state_period;
        }else{
            m_context.wait_until(next_tick);
            next_tick += m_state_period;
        }
    }
    m_context.throw_if_cancelled();

    m_active_command->stop_session_and_rethrow();
}



}
