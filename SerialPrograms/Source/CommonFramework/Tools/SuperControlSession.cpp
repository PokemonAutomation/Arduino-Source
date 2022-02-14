/*  Super-Control Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "ConsoleHandle.h"
#include "InterruptableCommands.h"
#include "SuperControlSession.h"

namespace PokemonAutomation{


SuperControlSession::SuperControlSession(ProgramEnvironment& env, ConsoleHandle& console)
    : m_env(env)
    , m_console(console)
{}

void SuperControlSession::run_session(){
//    AsyncAudioInferenceSession audio(m_env, m_console, m_console, m_console);
    AsyncVisualInferenceSession visual(m_env, m_console, m_console, m_console);
    for (VisualInferenceCallback* callback : m_visual_callbacks){
        visual += *callback;
    }

    AsyncCommandSession commands(m_env, m_console.botbase());

    while (!run_state(commands));

    commands.stop_session();
    visual.stop();
//    audio.stop();
}



}
