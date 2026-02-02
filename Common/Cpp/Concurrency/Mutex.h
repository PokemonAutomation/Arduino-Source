/*  Mutex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Concurrency_Mutex_H
#define PokemonAutomation_Concurrency_Mutex_H

#include "Qt6.9ThreadBugWorkaround.h"

#ifdef PA_ENABLE_QT_ADOPTION_WORKAROUND

#include <mutex>
#include <QMutex>
namespace PokemonAutomation{
    using Mutex = QMutex;
}

#else

#include <mutex>
namespace PokemonAutomation{
    using Mutex = std::mutex;
}

#endif

#endif
