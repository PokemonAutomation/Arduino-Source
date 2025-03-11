/*  Max Lair Detect Pokemon Swap Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonSwapMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonSwapMenu_H

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/FailureWatchdog.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class PokemonSwapMenuDetector : public VisualInferenceCallback{
public:
    PokemonSwapMenuDetector(bool stop_no_detect);

    bool detect(const ImageViewRGB32& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;

private:
    bool m_stop_on_no_detect;
    ImageFloatBox m_pink0;
    ImageFloatBox m_pink1;
    ImageFloatBox m_pink2;
    ImageFloatBox m_white0;
    ImageFloatBox m_white1;
//    ImageFloatBox m_bottom;
//    ImageFloatBox m_box0;
//    ImageFloatBox m_box1;
    ImageFloatBox m_bottom_main;
    ImageFloatBox m_bottom_right;
};



struct PokemonSwapMenuState{
    //  Index of your current selection.
    //  -1 means it's not your turn to choose.
    int index = -1;

    //  Pokemon selection options.
    std::string option[2];
};

class PokemonSwapMenuReader{
public:
    PokemonSwapMenuReader(
        Logger& logger,
        VideoOverlay& overlay,
        Language language,
        OcrFailureWatchdog& ocr_watchdog
    );

    bool my_turn(const ImageViewRGB32& screen);

    void read_options(const ImageViewRGB32& screen, std::string option[2]);
    void read_hp(const ImageViewRGB32& screen, double hp[4]);
    void read_pp(const ImageViewRGB32& screen, int8_t pp[4]);


private:
    Logger& m_logger;
    Language m_language;
    OcrFailureWatchdog& m_ocr_watchdog;
    OverlayBoxScope m_sprite0;
    OverlayBoxScope m_sprite1;
    OverlayBoxScope m_name0;
    OverlayBoxScope m_name1;
    OverlayBoxScope m_select0;
    OverlayBoxScope m_select1;
    OverlayBoxScope m_pp0;
    OverlayBoxScope m_pp1;
    OverlayBoxScope m_pp2;
    OverlayBoxScope m_pp3;
    OverlayBoxScope m_hp0;
    OverlayBoxScope m_hp1;
    OverlayBoxScope m_hp2;
    OverlayBoxScope m_hp3;
};



}
}
}
}
#endif
