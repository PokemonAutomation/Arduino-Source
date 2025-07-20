/*  Max Lair Detect Path Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSelect_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


class PathScreenDetector : public VisualInferenceCallback{
public:
    PathScreenDetector();

    bool detect(const ImageViewRGB32& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

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

    bool detect(const ImageViewRGB32& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;


private:
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_dialog_left;
    ImageFloatBox m_dialog_middle;
//    ImageFloatBox m_dialog_right;
    ImageFloatBox m_left;
    ImageFloatBox m_path_box;
};




class PathReader{
public:
    PathReader(VideoOverlay& overlay, size_t player_index);

    void read_sprites(
        Logger& logger,
        GlobalState& state,
        const ImageViewRGB32& screen
    ) const;
    void read_hp(
        Logger& logger,
        GlobalState& state,
        const ImageViewRGB32& screen
    ) const;


    void read_path(
        ProgramEnvironment& env,
        VideoStream& stream, ProControllerContext& context,
        GlobalState& state
    );


    //  Determine which side you're on (left or right) in the path.
    //      -1  =   unknown
    //      0   =   left
    //      1   =   right
    void read_side(
        Logger& logger,
        GlobalState& state,
        const ImageViewRGB32& screen
    );


public:
    void read_sprites(Logger& logger, const ImageViewRGB32& screen, std::string slugs[4]) const;
    void read_hp(Logger& logger, const ImageViewRGB32& screen, double hp[4]) const;

private:
//    size_t m_player_index;
    OverlayBoxScope m_path;
    OverlayBoxScope m_sprite0;
    OverlayBoxScope m_sprite1;
    OverlayBoxScope m_sprite2;
    OverlayBoxScope m_sprite3;
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
