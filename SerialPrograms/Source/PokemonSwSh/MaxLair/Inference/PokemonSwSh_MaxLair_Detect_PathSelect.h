/*  Max Lair Detect Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSelect_H

#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


class PathSelectDetector : public VisualInferenceCallback{
public:
    PathSelectDetector();

    bool detect(const QImage& screen) const;

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    ImageFloatBox m_bottom_main;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_dialog_left;
    ImageFloatBox m_dialog_middle;
//    ImageFloatBox m_dialog_right;
    ImageFloatBox m_main;
    ImageFloatBox m_left;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    ImageFloatBox m_box2;
    ImageFloatBox m_box3;
};



bool read_path(Path& path, ConsoleHandle& console);


class PathPartyReader{
public:
    PathPartyReader(VideoOverlay& overlay, size_t player_index);

    void read_sprites(
        Logger& logger,
        ConsoleHandle& console,
        GlobalState& state,
        const QImage& screen
    ) const;
    void read_hp(
        Logger& logger,
        ConsoleHandle& console,
        GlobalState& state,
        const QImage& screen
    ) const;

public:
    void read_sprites(Logger& logger, const QImage& screen, std::string slugs[4]) const;
    void read_hp(Logger& logger, const QImage& screen, double hp[4]) const;

private:
    size_t m_player_index;
    InferenceBoxScope m_sprite0;
    InferenceBoxScope m_sprite1;
    InferenceBoxScope m_sprite2;
    InferenceBoxScope m_sprite3;
    InferenceBoxScope m_hp0;
    InferenceBoxScope m_hp1;
    InferenceBoxScope m_hp2;
    InferenceBoxScope m_hp3;
};




}
}
}
}
#endif
