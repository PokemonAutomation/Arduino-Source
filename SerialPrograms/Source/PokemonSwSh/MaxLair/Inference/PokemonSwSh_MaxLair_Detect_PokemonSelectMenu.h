/*  Max Lair Detect Pokemon Select Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonSelectMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonSelectMenu_H

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/FailureWatchdog.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    struct ImageStats;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class PokemonSelectMenuDetector : public VisualInferenceCallback{
public:
    PokemonSelectMenuDetector(bool stop_no_detect);

    bool detect(const ImageViewRGB32& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;


private:
    static bool is_pink(const ImageStats& stats);


private:
    bool m_stop_on_no_detect;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    ImageFloatBox m_box2;
    ImageFloatBox m_box3;
//    ImageFloatBox m_box4;
//    ImageFloatBox m_box5;
//    ImageFloatBox m_box6;
    ImageFloatBox m_select0;
    ImageFloatBox m_select1;
    ImageFloatBox m_select2;
};



struct PokemonSelectMenuState{
    //  Index of your current selection.
    //  -1 means it's not your turn to choose.
    int index = -1;

    //  Pokemon selection options.
    std::string option[3];
};

class PokemonSelectMenuReader{
public:
    PokemonSelectMenuReader(
        Logger& logger,
        VideoOverlay& overlay,
        Language language,
        OcrFailureWatchdog& ocr_watchdog
    );

    int8_t who_is_selecting(const ImageViewRGB32& screen) const;

    std::string read_option(const ImageViewRGB32& screen, size_t index);
//    void read_options(const ImageViewRGB32& screen, std::string option[3]);


private:
    Logger& m_logger;
    Language m_language;
    OcrFailureWatchdog& m_ocr_watchdog;
    OverlayBoxScope m_sprite0;
    OverlayBoxScope m_sprite1;
    OverlayBoxScope m_sprite2;
    OverlayBoxScope m_name0;
    OverlayBoxScope m_name1;
    OverlayBoxScope m_name2;
    OverlayBoxScope m_player0;
    OverlayBoxScope m_player1;
    OverlayBoxScope m_player2;
    OverlayBoxScope m_player3;
};



}
}
}
}
#endif
