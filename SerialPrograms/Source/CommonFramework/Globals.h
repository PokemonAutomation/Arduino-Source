/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Globals_H
#define PokemonAutomation_Globals_H

#include <chrono>
#include <QString>

namespace PokemonAutomation{

extern const QString VERSION;
extern const QString DISCORD_LINK;
extern const QString DISCORD_LINK_FULL;
extern const QString GITHUB_REPO;

const auto SERIAL_REFRESH_RATE = std::chrono::milliseconds(1000);
extern const QString STRING_POKEMON;




enum class ProgramState{
    STOPPED,
    RUNNING,
    FINISHED,
    STOPPING,
};

enum class FeedbackType{
    NONE,
    OPTIONAL_,  //  Naming conflict with macro.
    REQUIRED,
};


#if 0
template <typename Lambda>
void run_on_main_thread(std::atomic<size_t>& reference_counter, Lambda&& lambda){
    reference_counter.fetch_add(1);
    QMetaObject::invokeMethod(
        application.get(),
        [&, lambda = std::move(lambda)]{
            lambda();
            reference_counter.fetch_sub(1);
        }
    );
}
#endif



}
#endif
