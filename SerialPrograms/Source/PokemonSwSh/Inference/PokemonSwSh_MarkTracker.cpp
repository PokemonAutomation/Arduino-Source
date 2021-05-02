/*  Mark Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh_MarkTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



MarkTracker::MarkTracker(ProgramEnvironment& env, ConsoleHandle& console)
    : m_env(env)
    , m_console(console)
    , m_start_battle(console, std::chrono::milliseconds(0))
    , m_battle_menu(console)
    , m_search_area(console, 0.0, 0.2, 1.0, 0.8)
{}

MarkWatchResult MarkTracker::detect_now(
    std::vector<InferenceBox>& exclamations,
    std::vector<InferenceBox>& questions
){
    QImage screen = m_console.video().snapshot();

    //  Check if a battle has started.
    if ( m_battle_menu.detect(screen)){
        return MarkWatchResult::BATTLE_MENU;
    }
    if ( m_start_battle.detect(screen)){
        return MarkWatchResult::BATTLE_START;
    }

    //  Look for exclamation points and question marks.
    QImage search_image = extract_box(screen, m_search_area);

    std::vector<PixelBox> exclamation_marks;
    std::vector<PixelBox> question_marks;
    find_marks(
        search_image,
        &exclamation_marks,
        &question_marks
    );

    m_detection_boxes.clear();
    for (const PixelBox& mark : exclamation_marks){
        InferenceBox box = translate_to_parent(screen, m_search_area, mark);
        box.color = Qt::magenta;
        box.x -= box.width;
        box.width *= 3;
        box.height *= 1.5;
        exclamations.emplace_back(box);
        m_detection_boxes.emplace_back(m_console, box);
//        cout << "asdf = " << exclamations.size() << endl;
    }
    for (const PixelBox& mark : question_marks){
        InferenceBox box = translate_to_parent(screen, m_search_area, mark);
        box.color = Qt::magenta;
        box.x -= box.width / 2;
        box.width *= 2;
        box.height *= 1.5;
        questions.emplace_back(box);
        m_detection_boxes.emplace_back(m_console, box);
//        cout << "qwer = " << questions.size() << endl;
    }

    return MarkWatchResult::NO_BATTLE;
}

MarkWatchResult MarkTracker::watch_for(
    std::vector<InferenceBox>& exclamations,
    std::vector<InferenceBox>& questions,
    std::chrono::milliseconds duration
){
    InferenceThrottler throttler(duration, std::chrono::milliseconds(50));
    while (true){
        m_env.check_stopping();

        MarkWatchResult result = detect_now(exclamations, questions);
        if (result != MarkWatchResult::NO_BATTLE){
            return result;
        }

        if (throttler.end_iteration(m_env)){
            return MarkWatchResult::NO_BATTLE;
        }
    }
}


AsyncMarkTracker::AsyncMarkTracker(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::vector<InferenceBox>& exclamations,
    std::vector<InferenceBox>& questions
)
    : MarkTracker(env, console)
    , m_stopping(false)
    , m_result(MarkWatchResult::NO_BATTLE)
    , m_thread(
        &AsyncMarkTracker::thread_loop, this,
        std::ref(exclamations),
        std::ref(questions)
    )
{}
AsyncMarkTracker::~AsyncMarkTracker(){
    m_stopping.store(true, std::memory_order_release);
    m_thread.join();
}

MarkWatchResult AsyncMarkTracker::result() const{
    return m_result.load(std::memory_order_acquire);
}



void AsyncMarkTracker::thread_loop(
    std::vector<InferenceBox>& exclamations,
    std::vector<InferenceBox>& questions
){
    InferenceThrottler throttler(std::chrono::milliseconds(0), std::chrono::milliseconds(50));
    try{
        while (!m_stopping.load(std::memory_order_acquire)){
            m_env.check_stopping();

            MarkWatchResult result = detect_now(exclamations, questions);
            if (result != MarkWatchResult::NO_BATTLE){
                m_result.store(result, std::memory_order_release);
                return;
            }

            throttler.end_iteration(m_env);
        }
    }catch (CancelledException&){}
}











}
}
}

