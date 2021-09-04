/*  Max Lair Detect Pokemon Swap Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonSwapMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonSwapMenu_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class PokemonSwapMenuDetector : public VisualInferenceCallbackWithCommandStop{
public:
    PokemonSwapMenuDetector(VideoOverlay& overlay, bool stop_no_detect);

    bool detect(const QImage& screen) const;

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;

private:
    bool m_stop_on_no_detect;
    InferenceBoxScope m_pink0;
    InferenceBoxScope m_pink1;
    InferenceBoxScope m_white;
//    InferenceBoxScope m_bottom;
//    InferenceBoxScope m_box0;
//    InferenceBoxScope m_box1;
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
        Language language
    );

    bool my_turn(const QImage& screen);

    void read_options(const QImage& screen, std::string option[2]);
    void read_pp(const QImage& screen, int8_t pp[4]);


private:
    Logger& m_logger;
    Language m_language;
    InferenceBoxScope m_sprite0;
    InferenceBoxScope m_sprite1;
    InferenceBoxScope m_name0;
    InferenceBoxScope m_name1;
    InferenceBoxScope m_select0;
    InferenceBoxScope m_select1;
    InferenceBoxScope m_pp0;
    InferenceBoxScope m_pp1;
    InferenceBoxScope m_pp2;
    InferenceBoxScope m_pp3;
};



}
}
}
}
#endif
