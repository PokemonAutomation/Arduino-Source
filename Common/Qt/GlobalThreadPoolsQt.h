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


//  Temporary and only used for audio template loading.
//  Hopefully we can remove this in the near future.
QtWorkerThreadPool& qt_worker_threadpool();


QtEventThreadPool& qt_event_threadpool();



}
}
#endif
