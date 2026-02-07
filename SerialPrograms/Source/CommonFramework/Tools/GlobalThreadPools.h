/*  Global Thread Pools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_GlobalThreadPools_H
#define PokemonAutomation_CommonTools_GlobalThreadPools_H

#include "Common/Cpp/Concurrency/ComputationThreadPool.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{


//  Computational thread pools.
//  These are capped to their respective MAX_THREADS and may block.
ComputationThreadPool& computation_realtime();
ComputationThreadPool& computation_normal();

//  These are general purpose thread pools of unlimited size.
//  Dispatching to these will never block.
ComputationThreadPool& unlimited_realtime();
ComputationThreadPool& unlimited_pivot();
ComputationThreadPool& unlimited_normal();



}
}
#endif
