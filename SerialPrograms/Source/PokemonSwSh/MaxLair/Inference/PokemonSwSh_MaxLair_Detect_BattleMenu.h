/*  Max Lair Detect Battle Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_BattleMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_BattleMenu_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/FailureWatchdog.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class BattleMenuDetector : public VisualInferenceCallback{
public:
    BattleMenuDetector();

    bool detect(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;

    bool dmaxed() const{ return m_dmaxed; }
    bool cheer() const{ return m_cheer; }


private:
    ImageFloatBox m_icon_fight;
    ImageFloatBox m_icon_pokemon;
    ImageFloatBox m_icon_run;
    ImageFloatBox m_text_fight;
    ImageFloatBox m_text_pokemon;
    ImageFloatBox m_text_run;
    ImageFloatBox m_icon_cheer; //  Specific for Italian
//    ImageFloatBox m_info_left;
//    ImageFloatBox m_info_right;
    ImageFloatBox m_status0;
    ImageFloatBox m_status1;

    size_t m_trigger_count = 0;
    bool m_dmaxed = false;
    bool m_cheer = false;
};



class BattleMenuReader{
public:
    BattleMenuReader(
        VideoOverlay& overlay,
        Language language,
        OcrFailureWatchdog& ocr_watchdog
    );

    std::set<std::string> read_opponent(
        Logger& logger, CancellableScope& scope,
        VideoFeed& feed
    ) const;
    std::set<std::string> read_opponent_in_summary(Logger& logger, const ImageViewRGB32& screen) const;

    std::string read_own_mon(Logger& logger, const ImageViewRGB32& screen) const;

    double read_opponent_hp(Logger& logger, const ImageViewRGB32& screen) const;
    double read_own_hp(Logger& logger, const ImageViewRGB32& screen) const;
    void read_hp(Logger& logger, const ImageViewRGB32& screen, Health health[4], size_t player_index);
    void read_own_pp(Logger& logger, const ImageViewRGB32& screen, int8_t pp[4]) const;
    bool can_dmax(const ImageViewRGB32& screen) const;

private:
    Language m_language;
    OcrFailureWatchdog& m_ocr_watchdog;
    OverlayBoxScope m_opponent_name;
    OverlayBoxScope m_summary_opponent_name;
    OverlayBoxScope m_summary_opponent_types;

    OverlayBoxScope m_own_name;
    OverlayBoxScope m_own_sprite;

    OverlayBoxScope m_opponent_hp;
    OverlayBoxScope m_own_hp;

    OverlayBoxScope m_hp0;
    OverlayBoxScope m_hp1;
    OverlayBoxScope m_hp2;
    OverlayBoxScope m_sprite0;
    OverlayBoxScope m_sprite1;
    OverlayBoxScope m_sprite2;

    OverlayBoxScope m_pp0;
    OverlayBoxScope m_pp1;
    OverlayBoxScope m_pp2;
    OverlayBoxScope m_pp3;
    OverlayBoxScope m_dmax;
};



}
}
}
}
#endif
