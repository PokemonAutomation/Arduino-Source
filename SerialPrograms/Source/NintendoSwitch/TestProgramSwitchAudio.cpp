/*  Test Program Switch Audio
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/InferenceException.h"
#include "CommonFramework/Tools/AudioFeed.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "TestProgramSwitchAudio.h"

#include <set>
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{


TestProgramAudio_Descriptor::TestProgramAudio_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "NintendoSwitch:TestProgramAudio",
        "Nintendo Switch", "Test Program Audio",
        "",
        "Test Program (Switch Audio)",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


TestProgramAudio::TestProgramAudio(const TestProgramAudio_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
}


struct TestProgramAudio::Stats : public StatsTracker{
    Stats() {}
};

std::unique_ptr<StatsTracker> TestProgramAudio::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void TestProgramAudio::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_move_right_joystick(env.console, 0, 255, 10, 0);

    std::cout << "Running audio test program." << std::endl;

    // virtual void spectrums_latest(size_t numLatestSpectrums, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums) = 0;
    std::vector<std::shared_ptr<AudioSpectrum>> spectrums;
    env.console.audio().spectrums_latest(3, spectrums);
    for(auto p : spectrums){
        std::cout << "Spectrum: " << p->stamp << std::endl;
        for(size_t i = 0; i < 10; i++){
            std::cout << p->magnitudes[i] << " ";
        }
        std::cout << std::endl;
    }


    std::cout << "Audio test program finished." << std::endl;

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}





}
}
