/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Globals.h"

namespace PokemonAutomation{


//
//  ATTENTION!!!
//
//  If you are building from source, do not change any of these version numbers
//  or tags. When filing reports or asking for support, we (the developers) need
//  to know exactly what you are running. Changing these values can lead to
//  misleading version information.
//

#ifndef PA_IS_BETA
#define PA_IS_BETA true
#endif

#ifndef PA_VERSION_MAJOR
#define PA_VERSION_MAJOR 0
#endif

#ifndef PA_VERSION_MINOR
#define PA_VERSION_MINOR 69
#endif

#ifndef PA_VERSION_PATCH
#define PA_VERSION_PATCH 16
#endif

const bool IS_BETA_VERSION = PA_IS_BETA;
const int PROGRAM_VERSION_MAJOR = PA_VERSION_MAJOR;
const int PROGRAM_VERSION_MINOR = PA_VERSION_MINOR;
const int PROGRAM_VERSION_PATCH = PA_VERSION_PATCH;

const std::string PROGRAM_VERSION_BASE =
    "v" + std::to_string(PROGRAM_VERSION_MAJOR) +
    "." + std::to_string(PROGRAM_VERSION_MINOR) +
    "." + std::to_string(PROGRAM_VERSION_PATCH);

#ifdef PA_OFFICIAL
const std::string PROGRAM_VERSION = IS_BETA_VERSION
    ? PROGRAM_VERSION_BASE + "-beta"
    : PROGRAM_VERSION_BASE;
#else
const std::string PROGRAM_VERSION = PROGRAM_VERSION_BASE + "-user";
#endif



const std::string PROGRAM_NAME = "Pok\u00e9mon Automation";

const std::string ONLINE_DOC_URL_BASE = "https://pokemonautomation.github.io/";
const std::string PROJECT_SOURCE_URL = "https://github.com/PokemonAutomation/Arduino-Source/";
const std::string RESOURCES_URL_BASE = "https://github.com/PokemonAutomation/Packages/";



//  This the URL that we display. We don't actually use this for linking.
const std::string GITHUB_LINK_TEXT = "pokemonautomation.github.io";

//  This is the URL that we actually link to.
const std::string GITHUB_LINK_URL = "https://pokemonautomation.github.io";



//  URL to display. (the vanity link)
//  We don't actually use this URL for linking since the vanity link will go
//  away if we lose too many nitro boosts.
const std::string DISCORD_LINK_TEXT = "discord.gg/PokemonAutomation";

//  URL to use inside the program.
const std::string DISCORD_LINK_URL_PROGRAM = "https://discord.gg/BSjDp27";

//  URL to use in the Discord notifications/embeds.
const std::string DISCORD_LINK_URL_EMBED = "https://discord.gg/xMJcveK";

//  URL to use in the DiscordSocialSDK integration.
const std::string DISCORD_LINK_URL_SDK = "https://discord.gg/gn9YEyjjAV";



#if 0
#elif __INTEL_LLVM_COMPILER
const std::string COMPILER_VERSION = "ICX " + std::to_string(__VERSION__);
#elif __INTEL_COMPILER
const std::string COMPILER_VERSION = "ICC " + std::to_string(__INTEL_COMPILER) + "." + std::to_string(__INTEL_COMPILER_UPDATE);
#elif _MSC_VER
const std::string COMPILER_VERSION = "MSVC " + std::to_string(_MSC_FULL_VER);
#elif __clang__
const std::string COMPILER_VERSION = "Clang " + std::string(__clang_version__);
#elif __GNUC__
const std::string COMPILER_VERSION = "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#else
const std::string COMPILER_VERSION = "Unknown Compiler";
#endif



const size_t LOG_HISTORY_LINES = 10000;


}

