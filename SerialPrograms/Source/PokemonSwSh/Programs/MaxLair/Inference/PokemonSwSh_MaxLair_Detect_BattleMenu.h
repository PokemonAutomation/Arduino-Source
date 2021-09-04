/*  Max Lair Detect Battle Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_BattleMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_BattleMenu_H

#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class BattleMenuDetector : public VisualInferenceCallbackWithCommandStop{
public:
    BattleMenuDetector(VideoOverlay& overlay);

    bool detect(const QImage& screen);

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    InferenceBoxScope m_icon_fight;
    InferenceBoxScope m_icon_pokemon;
    InferenceBoxScope m_icon_run;
    InferenceBoxScope m_text_fight;
    InferenceBoxScope m_text_pokemon;
    InferenceBoxScope m_text_run;
//    InferenceBoxScope m_info_left;
//    InferenceBoxScope m_info_right;
    InferenceBoxScope m_status;

    bool m_cheer = false;
};



class BattleMenuReader{
public:
    BattleMenuReader(VideoOverlay& overlay);

    double read_opponent_hp(Logger& logger, const QImage& screen) const;

private:
    InferenceBoxScope m_opponent_hp;
};



}
}
}
}
#endif
