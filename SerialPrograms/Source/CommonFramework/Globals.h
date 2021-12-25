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

extern const QString PROGRAM_VERSION;
extern const QString DISCORD_LINK;
extern const QString DISCORD_LINK_URL;
extern const QString ONLINE_DOC_URL;
extern const QString PROJECT_GITHUB;
extern const QString PROJECT_GITHUB_URL;
extern const QString PROJECT_SOURCE_URL;

const auto SERIAL_REFRESH_RATE = std::chrono::milliseconds(1000);
extern const QString STRING_POKEBALL;
extern const QString STRING_POKEMON;
extern const QString STRING_POKEDEX;
extern const QString STRING_POKEJOB;

const QString& RESOURCE_PATH();
const QString& TRAINING_PATH();


enum class ProgramState{
    NOT_READY,
    STOPPED,
    RUNNING,
//    FINISHED,
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
