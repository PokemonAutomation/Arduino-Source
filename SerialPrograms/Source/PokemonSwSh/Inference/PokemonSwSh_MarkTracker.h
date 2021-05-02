/*  Mark Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MarkTracker_H
#define PokemonAutomation_PokemonSwSh_MarkTracker_H

#include <deque>
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


enum MarkWatchResult{
    NO_BATTLE,
    BATTLE_START,
    BATTLE_MENU,
};


class MarkTracker{
public:
    MarkTracker(ProgramEnvironment& env, ConsoleHandle& console);

    MarkWatchResult detect_now(
        std::vector<InferenceBox>& exclamations,
        std::vector<InferenceBox>& questions
    );
    MarkWatchResult watch_for(
        std::vector<InferenceBox>& exclamations,
        std::vector<InferenceBox>& questions,
        std::chrono::milliseconds duration
    );

protected:
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;
    StartBattleDetector m_start_battle;
    StandardBattleMenuDetector m_battle_menu;
    InferenceBoxScope m_search_area;
    std::deque<InferenceBoxScope> m_detection_boxes;
};



class AsyncMarkTracker : protected MarkTracker{
public:
    AsyncMarkTracker(
        ProgramEnvironment& env, ConsoleHandle& console,
        std::vector<InferenceBox>& exclamations,
        std::vector<InferenceBox>& questions
    );
    ~AsyncMarkTracker();
    MarkWatchResult result() const;
private:
    void thread_loop(
        std::vector<InferenceBox>& exclamations,
        std::vector<InferenceBox>& questions
    );
private:
    std::atomic<bool> m_stopping;
    std::atomic<MarkWatchResult> m_result;
    std::thread m_thread;
};




}
}
}
#endif

