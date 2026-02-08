/*  Global Thread Pools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_GlobalThreadPools_H
#define PokemonAutomation_CommonTools_GlobalThreadPools_H

#include "Common/Cpp/Concurrency/ThreadPool.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{


//  Computational thread pools.
//  These are capped to their respective MAX_THREADS and may block.
//  Do not use these for threads that like to idle.
ThreadPool& computation_realtime();
ThreadPool& computation_normal();

//  These are general purpose thread pools of unlimited size.
//  Dispatching to these will never block.
//  Use these for threads that like to idle.
ThreadPool& unlimited_realtime();
ThreadPool& unlimited_pivot();
ThreadPool& unlimited_normal();



}
}
#endif
