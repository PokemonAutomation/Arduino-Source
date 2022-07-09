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

extern const std::string UTF8_PROGRAM_VERSION;
extern const std::string UTF8_STRING_POKEMON;

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



}
#endif
