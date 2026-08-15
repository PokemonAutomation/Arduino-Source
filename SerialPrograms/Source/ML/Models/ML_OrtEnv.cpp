/*  Global Ort::Env
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "ML_OrtEnv.h"

namespace PokemonAutomation{



Ort::ThreadingOptions make_ort_threading_options(){
    Ort::ThreadingOptions ret;
    ret.SetGlobalIntraOpNumThreads((int)PerformanceOptions::instance().NORMAL_THREAD_POOL.MAX_THREADS);
    return ret;
}

Ort::Env& global_ort_env(){
#if ORT_API_VERSION < 24 // Removed in ONNX Runtime 1.24.0
    if (Ort::Global<void>::api_ == nullptr){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Onnx API returned a null pointer.");
    }
#endif
    static Ort::Env env(
        make_ort_threading_options(),
        ORT_LOGGING_LEVEL_WARNING,
        "Global_ThreadPool_Env"
    );
    return env;
}



}
