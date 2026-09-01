/*  Global Ort::Env
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ML_OrtEnv_H
#define PokemonAutomation_ML_OrtEnv_H

#include <onnxruntime_cxx_api.h>

namespace PokemonAutomation{


Ort::Env& global_ort_env();




}
#endif
