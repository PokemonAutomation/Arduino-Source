/*  Overworld Sensors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_OverworldSensors_H
#define PokemonAutomation_PokemonSV_OverworldSensors_H

#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceSession.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonSV/Inference/Battles/PokemonSV_StartBattleYellowBar.h"
#include "PokemonSV/Inference/Battles/PokemonSV_ShinySoundDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class ShinySoundTracker : public ShinySoundDetector, public VisualInferenceCallback{
public:
    ShinySoundTracker(Logger& logger);

    bool last_detection(
        WallClock& timestamp, float& error,
        VideoSnapshot& screenshot
    ) const;

    virtual void make_overlays(VideoOverlaySet& items) const override{}
    virtual bool process_frame(const VideoSnapshot& frame) override;

private:
    void on_shiny_detected(float error);

private:
    mutable SpinLock m_lock;

    WallClock m_last_detection;
    float m_last_error;
    VideoSnapshot m_last_screenshot;

    std::deque<VideoSnapshot> m_history;
};






class OverworldSensors{
public:
    using DetectedCallback = AudioPerSpectrumDetectorBase::DetectedCallback;

    OverworldSensors(
        Logger& logger, VideoStream& stream, CancellableScope& scope
    )
        : lets_go_hp(COLOR_RED)
        , lets_go_kill(logger)
        , start_battle(COLOR_RED)
        , shiny_sound(logger)
        , m_session(
            scope, stream,
            {
                lets_go_hp,
                lets_go_kill,
                start_battle,
                static_cast<AudioInferenceCallback&>(shiny_sound),
                static_cast<VisualInferenceCallback&>(shiny_sound),
//                shiny_screenshot,
            }
        )
    {
    }

    void throw_if_no_sound(Milliseconds min_duration = Milliseconds(10000)) const{
        lets_go_kill.throw_if_no_sound(min_duration);
        shiny_sound.throw_if_no_sound(min_duration);
    }

public:
    LetsGoHpWatcher lets_go_hp;
    LetsGoKillSoundDetector lets_go_kill;
    StartBattleYellowBarWatcher start_battle;
    ShinySoundTracker shiny_sound;
//    ShinyScreenshotTracker shiny_screenshot;


private:
    InferenceSession m_session;
};










class OverworldBattleTracker{
public:
    enum class Detection{
        CONTINUE_BATTLE,
        NEW_BATTLE,
        NEW_SHINY_BATTLE,
    };

public:
    OverworldBattleTracker(Logger& logger, OverworldSensors& sensors);

    void report_in_battle(){
        m_start_time = current_time();
        m_last_known_in_battle = true;
    }
    void report_out_of_battle(){
        m_start_time = current_time();
        m_last_known_in_battle = false;
    }

    Detection report_battle_menu_detected();
    Detection report_battle_menu_detected(VideoSnapshot& shiny_screenshot, float& shiny_error);

private:
    Logger& m_logger;
    OverworldSensors& m_sensors;
    WallClock m_start_time;
    bool m_last_known_in_battle;
};





















}
}
}
#endif
