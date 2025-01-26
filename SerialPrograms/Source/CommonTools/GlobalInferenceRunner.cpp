/*  Global Inference Runner
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "GlobalInferenceRunner.h"

namespace PokemonAutomation{



ParallelTaskRunner& global_inference_runner(){
    static ParallelTaskRunner runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->INFERENCE_PRIORITY.set_on_this_thread();
        },
        0, std::thread::hardware_concurrency()
    );
    return runner;
}



}
