/*  Global Thread Pools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  On Windows it is not safe to call into Qt from std::thread.
 *
 *  If we need to call into Qt signals/slots from std::thread, dispatch to this
 *  thread pool instead.
 *
 */

#ifndef PokemonAutomation_CommonTools_GlobalThreadPoolsQt_H
#define PokemonAutomation_CommonTools_GlobalThreadPoolsQt_H

#include "Common/Qt/QtThreadPool.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{



QWorkerThreadPool& qt_threadpool();



}
}
#endif
