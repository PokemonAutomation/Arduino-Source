/*  Global Ort::Env
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "ML_OrtEnv.h"

namespace PokemonAutomation{



Ort::ThreadingOptions make_ort_threading_options(){
    Ort::ThreadingOptions ret;
    {
        int threads = (int)PerformanceOptions::instance().ONNX_OPTIONS.MAX_INTRA_OP_THREADS;
        global_logger_tagged().log("Setting ONNX Intra-Op Threads: " + tostr_u_commas(threads));
        ret.SetGlobalIntraOpNumThreads(threads);
    }
    {
        int threads = (int)PerformanceOptions::instance().ONNX_OPTIONS.MAX_INTER_OP_THREADS;
        global_logger_tagged().log("Setting ONNX Inter-Op Threads: " + tostr_u_commas(threads));
        ret.SetGlobalInterOpNumThreads(threads);
    }
    ret.SetGlobalSpinControl(0);
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
