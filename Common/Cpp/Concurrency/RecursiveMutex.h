/*  Recursive Mutex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Concurrency_RecursiveMutex_H
#define PokemonAutomation_Concurrency_RecursiveMutex_H

#include "Qt6.9ThreadBugWorkaround.h"

//  Note from Kuro (2026/08/15): I think it's ok to disable this workaround for
//  mutex and conditional_variable since we're completely leaking the thread.
//  So the parasitic adoption should dangle these sync classes.
#if 0 //def PA_ENABLE_QT_ADOPTION_WORKAROUND

#include <QRecursiveMutex>
namespace PokemonAutomation{
    using RecursiveMutex = QRecursiveMutex;
}

#else

#include <mutex>
namespace PokemonAutomation{
    using RecursiveMutex = std::recursive_mutex;
}

#endif

#endif
