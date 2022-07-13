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

extern const std::string PROGRAM_VERSION;
extern const std::string DISCORD_LINK;
extern const std::string DISCORD_LINK_URL;
extern const std::string ONLINE_DOC_URL;
extern const std::string PROJECT_GITHUB;
extern const std::string PROJECT_GITHUB_URL;
extern const std::string PROJECT_SOURCE_URL;

const auto SERIAL_REFRESH_RATE = std::chrono::milliseconds(1000);
extern const QString STRING_POKEBALL;
extern const std::string STRING_POKEMON;
extern const std::string STRING_POKEDEX;
extern const std::string STRING_POKEJOB;

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
