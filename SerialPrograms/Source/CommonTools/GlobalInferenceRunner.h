/*  Global Inference Runner
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonTools_GlobalInferenceRunner_H
#define PokemonAutomation_CommonTools_GlobalInferenceRunner_H

#include "Common/Cpp/Concurrency/ParallelTaskRunner.h"

namespace PokemonAutomation{



ParallelTaskRunner& global_inference_runner();



}
#endif
