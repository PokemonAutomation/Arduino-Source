/*  Max Lair Detect Pokemon Select Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonSelectMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonSelectMenu_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

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
        LoggerQt& logger,
        VideoOverlay& overlay,
        Language language
    );

    int8_t who_is_selecting(const ImageViewRGB32& screen) const;

    std::string read_option(const ImageViewRGB32& screen, size_t index);
//    void read_options(const ImageViewRGB32& screen, std::string option[3]);


private:
    LoggerQt& m_logger;
    Language m_language;
    InferenceBoxScope m_sprite0;
    InferenceBoxScope m_sprite1;
    InferenceBoxScope m_sprite2;
    InferenceBoxScope m_name0;
    InferenceBoxScope m_name1;
    InferenceBoxScope m_name2;
    InferenceBoxScope m_player0;
    InferenceBoxScope m_player1;
    InferenceBoxScope m_player2;
    InferenceBoxScope m_player3;
};



}
}
}
}
#endif
