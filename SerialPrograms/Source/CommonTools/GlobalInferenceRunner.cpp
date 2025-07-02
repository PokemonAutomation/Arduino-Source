/*  Global Inference Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "GlobalInferenceRunner.h"

namespace PokemonAutomation{



ParallelTaskRunner& global_inference_runner(){
    static ParallelTaskRunner runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->NORMAL_INFERENCE_PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, std::thread::hardware_concurrency()
    );
    return runner;
}



}
