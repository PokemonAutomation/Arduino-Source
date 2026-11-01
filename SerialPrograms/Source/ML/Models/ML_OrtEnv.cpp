/*  Global Ort::Env
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <thread>
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
        std::cerr << "[ORT-DIAG] ONNX Runtime " << Ort::GetVersionString()
                  << ", hardware_concurrency() = " << std::thread::hardware_concurrency()
                  << ", SetGlobalIntraOpNumThreads(" << threads << ")" << std::endl;
        ret.SetGlobalIntraOpNumThreads(threads);
    }
    {
        int threads = (int)PerformanceOptions::instance().ONNX_OPTIONS.MAX_INTER_OP_THREADS;
        global_logger_tagged().log("Setting ONNX Inter-Op Threads: " + tostr_u_commas(threads));
        std::cerr << "[ORT-DIAG] SetGlobalInterOpNumThreads(" << threads << ")" << std::endl;
        ret.SetGlobalInterOpNumThreads(threads);
    }
    ret.SetGlobalSpinControl(0);
    //  Flush denormal floats to zero on ONNX Runtime's own threads: the big models produce
    //  them, and they are very slow on Intel CPUs. ONNX Runtime only supports this on x86;
    //  see Common/Cpp/Hardware/FloatingPointMode.h.
    ret.SetGlobalDenormalAsZero();
    std::cerr << "[ORT-DIAG] SetGlobalSpinControl(0), SetGlobalDenormalAsZero()" << std::endl;
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
        ORT_LOGGING_LEVEL_INFO,  //  [ORT-DIAG] was WARNING
        "Global_ThreadPool_Env"
    );
    std::cerr << "[ORT-DIAG] Ort::Env created" << std::endl;
    return env;
}



}
