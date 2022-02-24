/*  Super-Control Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "ConsoleHandle.h"
#include "InterruptableCommands.h"
#include "SuperControlSession.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


SuperControlSession::SuperControlSession(ProgramEnvironment& env, ConsoleHandle& console)
    : m_env(env)
    , m_console(console)
{}

void SuperControlSession::run_session(){
#if 0
    std::unique_ptr<AsyncAudioInferenceSession> visual;
    if (!m_visual_callbacks.empty()){
        visual.reset(new AsyncAudioInferenceSession(m_env, m_console, m_console, m_console));
        for (AudioInferenceCallback* callback : m_visual_callbacks){
            *visual += *callback;
        }
    }
#endif

    std::unique_ptr<AsyncVisualInferenceSession> visual;
    if (!m_visual_callbacks.empty()){
        visual.reset(new AsyncVisualInferenceSession(m_env, m_console, m_console, m_console));
        for (VisualInferenceCallback* callback : m_visual_callbacks){
            *visual += *callback;
        }
    }

    AsyncCommandSession commands(m_env, m_console.botbase());

    while (!run_state(commands));

//    cout << "SuperControlSession::run_session() - stop" << endl;
    commands.stop_session();

    if (visual){
        visual->stop();
    }

#if 0
    if (audio){
        audio->stop();
    }
#endif
//    cout << "SuperControlSession::run_session() - end" << endl;
}



}
