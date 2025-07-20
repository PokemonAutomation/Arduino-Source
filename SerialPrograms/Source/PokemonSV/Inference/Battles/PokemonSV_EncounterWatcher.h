/*  Encounter Watcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_EncounterWatcher_H
#define PokemonAutomation_PokemonSV_EncounterWatcher_H

#include <deque>
#include <mutex>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "PokemonSV_NormalBattleMenus.h"
#include "PokemonSV_ShinySoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

//
//  This class is now deprecated.
//
class EncounterWatcher : public VisualInferenceCallback, public AudioInferenceCallback{
public:
    EncounterWatcher(VideoStream& stream, Color color = COLOR_RED);

    const NormalBattleMenuDetector& battle_menu_detector() const{ return m_battle_menu; }

    const VideoSnapshot& shiny_screenshot() const{
        return m_best_snapshot;
    }
    float lowest_error_coefficient() const{
        return m_shiny_sound.lowest_error();
    }
    void throw_if_no_sound() const{
        m_shiny_sound.throw_if_no_sound();
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const VideoSnapshot& frame) override;
    virtual bool process_spectrums(
        const std::vector<AudioSpectrum>& new_spectrums,
        AudioFeed& audioFeed
    ) override;

private:
    NormalBattleMenuWatcher m_battle_menu;
    ShinySoundDetector m_shiny_sound;

    std::mutex m_lock;
    std::deque<VideoSnapshot> m_history;
    VideoSnapshot m_best_snapshot;
};




}
}
}
#endif
