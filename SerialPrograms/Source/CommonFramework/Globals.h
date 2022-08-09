/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Globals_H
#define PokemonAutomation_Globals_H

#include <chrono>
#include <string>

namespace PokemonAutomation{

extern const std::string PROGRAM_VERSION;
extern const std::string PROGRAM_NAME;

extern const std::string DISCORD_LINK;
extern const std::string DISCORD_LINK_URL;
extern const std::string ONLINE_DOC_URL;
extern const std::string PROJECT_GITHUB;
extern const std::string PROJECT_GITHUB_URL;
extern const std::string PROJECT_SOURCE_URL;

const auto SERIAL_REFRESH_RATE = std::chrono::milliseconds(1000);

const std::string& RESOURCE_PATH();
const std::string& TRAINING_PATH();


enum class ProgramState{
    NOT_READY,
    STOPPED,
    RUNNING,
    STOPPING,
};

enum class FeedbackType{
    NONE,
    OPTIONAL_,  //  Naming conflict with macro.
    REQUIRED,
};



}
#endif
