/*  Super-Control Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_SuperControlSession_H
#define PokemonAutomation_CommonFramework_SuperControlSession_H

#include <vector>

namespace PokemonAutomation{

class ProgramEnvironment;
class ConsoleHandle;
class AsyncCommandSession;
class AudioInferenceCallback;
class VisualInferenceCallback;


class SuperControlSession{
public:
    void run_session();

protected:
    SuperControlSession(ProgramEnvironment& env, ConsoleHandle& console);

protected:
    //  Return true if we should stop.
    virtual bool run_state(AsyncCommandSession& commands) = 0;

protected:
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;

    std::vector<AudioInferenceCallback*> m_audio_callbacks;
    std::vector<VisualInferenceCallback*> m_visual_callbacks;
};



}
#endif
