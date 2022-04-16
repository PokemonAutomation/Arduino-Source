/*  Max Lair Detect Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSelect_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


class PathScreenDetector : public VisualInferenceCallback{
public:
    PathScreenDetector();

    bool detect(const QImage& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_bottom_main;
    ImageFloatBox m_main;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    ImageFloatBox m_box2;
    ImageFloatBox m_box3;
};


class PathSelectDetector : public PathScreenDetector{
public:
    PathSelectDetector();

    bool detect(const QImage& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override final;


private:
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_dialog_left;
    ImageFloatBox m_dialog_middle;
//    ImageFloatBox m_dialog_right;
    ImageFloatBox m_left;
};




class PathReader{
public:
    PathReader(VideoOverlay& overlay, size_t player_index);

    void read_sprites(
        LoggerQt& logger,
        GlobalState& state,
        const QImage& screen
    ) const;
    void read_hp(
        LoggerQt& logger,
        GlobalState& state,
        const QImage& screen
    ) const;


    void read_path(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, GlobalState& state);


    //  Determine which side you're on (left or right) in the path.
    //      -1  =   unknown
    //      0   =   left
    //      1   =   right
    void read_side(
        LoggerQt& logger,
        GlobalState& state,
        const QImage& screen
    );


public:
    void read_sprites(LoggerQt& logger, const QImage& screen, std::string slugs[4]) const;
    void read_hp(LoggerQt& logger, const QImage& screen, double hp[4]) const;

    static int8_t read_side(const QImage& image, int p_min_rgb_sum);

private:
    size_t m_player_index;
    InferenceBoxScope m_path;
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
