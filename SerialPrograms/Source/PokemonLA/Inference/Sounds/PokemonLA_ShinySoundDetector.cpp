/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Recording/StreamHistorySession.h"
#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "CommonTools/Audio/AudioTemplateCache.h"
#include "Tests/TestUtils.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WiredController.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_ShinySoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


ShinySoundDetector::ShinySoundDetector(Logger& logger, DetectedCallback detected_callback)
    // Use a yellow as the detection color because the shiny animation is yellow.
    : AudioPerSpectrumDetectorBase(
        logger,
        "ShinySoundDetector",
        "Shiny sound",
        COLOR_YELLOW,
        detected_callback
    )
{}


float ShinySoundDetector::get_score_threshold() const{
    return (float)GameSettings::instance().SHINY_SOUND_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> ShinySoundDetector::build_spectrogram_matcher(size_t sample_rate){
    return std::make_unique<SpectrogramMatcher>(
        "Shiny Sound",
        AudioTemplateCache::instance().get_throw("PokemonLA/ShinySound", sample_rate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sample_rate,
        GameSettings::instance().SHINY_SOUND_LOW_FREQUENCY
    );
}




class Test_ShinySoundDetector : public UnitTest{
public:
    Test_ShinySoundDetector(std::vector<AudioSpectrum> spectrums, bool expected)
        : UnitTest("PokemonLA::ShinySoundDetector")
        , m_spectrums(std::move(spectrums))
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyBotBase botbase(logger);
        SerialPABotBase::SerialPABotBase_Connection connection(logger, "");
        SerialPABotBase_WiredController controller(
            logger, connection,
            ControllerType::NintendoSwitch_WiredController
        );
        DummyVideoFeed video_feed;
        DummyVideoOverlay video_overlay;
        DummyAudioFeed audio_feed;
        StreamHistorySession history(logger);

        ConsoleHandle console(0, logger, controller, video_feed, video_overlay, audio_feed, history);
        ShinySoundDetector detector(console, [&](float error_coefficient) -> bool{
            return true;
        });

        bool result = detector.process_spectrums(m_spectrums, audio_feed);
        TEST_RESULT_EQUAL_STR(result, m_expected);
        return true;
    };

private:
    std::vector<AudioSpectrum> m_spectrums;
    bool m_expected;
};


void add_tests_ShinySoundDetector(UnitTestDatabase& database){

}



}
}
}
