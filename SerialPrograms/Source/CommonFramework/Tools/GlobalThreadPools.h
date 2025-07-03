/*  Global Thread Pools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_GlobalThreadPools_H
#define PokemonAutomation_CommonTools_GlobalThreadPools_H

#include "Common/Cpp/Concurrency/ParallelTaskRunner.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{



ParallelTaskRunner& realtime_inference();
ParallelTaskRunner& normal_inference();



}
}
#endif
