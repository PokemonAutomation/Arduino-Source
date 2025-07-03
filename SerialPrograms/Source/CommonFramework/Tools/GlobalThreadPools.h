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



ComputationThreadPool& realtime_inference();
ComputationThreadPool& normal_inference();



}
}
#endif
