/*  Mutex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Concurrency_Qt69ThreadBugWorkaround_H
#define PokemonAutomation_Concurrency_Qt69ThreadBugWorkaround_H

//#define PA_ENABLE_QT_ADOPTION_WORKAROUND


#ifdef QT_CORE_LIB

#include <QtGlobal>

#if _WIN32 && (QT_VERSION_MAJOR * 1000000 + QT_VERSION_MINOR * 1000 + QT_VERSION_PATCH) > 6008003

#define PA_ENABLE_QT_ADOPTION_WORKAROUND

#endif

#endif


#endif
