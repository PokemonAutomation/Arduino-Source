/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Globals_H
#define PokemonAutomation_Globals_H

#include <string>

namespace PokemonAutomation{

extern const bool IS_BETA_VERSION;

extern const int PROGRAM_VERSION_MAJOR;
extern const int PROGRAM_VERSION_MINOR;
extern const int PROGRAM_VERSION_PATCH;
extern const std::string PROGRAM_VERSION_BASE;
extern const std::string PROGRAM_VERSION;

extern const std::string PROGRAM_NAME;

extern const std::string ONLINE_DOC_URL_BASE;
extern const std::string PROJECT_SOURCE_URL;
extern const std::string RESOURCES_URL_BASE;

extern const std::string GITHUB_LINK_TEXT;
extern const std::string GITHUB_LINK_URL;

extern const std::string DISCORD_LINK_TEXT;
extern const std::string DISCORD_LINK_URL_PROGRAM;
extern const std::string DISCORD_LINK_URL_EMBED;
extern const std::string DISCORD_LINK_URL_SDK;

extern const std::string COMPILER_VERSION;

extern const size_t LOG_HISTORY_LINES;

extern bool USE_QT_UI;



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
    VIDEO_AUDIO,
};



}
#endif
