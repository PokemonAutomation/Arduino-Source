/*  Tera Battle Menus
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraBattleMenus_H
#define PokemonAutomation_PokemonSV_TeraBattleMenus_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class TeraBattleMenuDetector : public StaticScreenDetector{
public:
    TeraBattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Returns -1 if not found.
    int8_t detect_slot(const ImageViewRGB32& screen) const;
    bool move_to_slot(VideoStream& stream, SwitchControllerContext& context, uint8_t slot) const;

private:
    WhiteButtonDetector m_callouts_button;
    GradientArrowDetector m_arrow;
};
class TeraBattleMenuWatcher : public DetectorToFinder<TeraBattleMenuDetector>{
public:
    TeraBattleMenuWatcher(Color color)
        : DetectorToFinder("TeraBattleMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};




class CheerSelectDetector : public StaticScreenDetector{
public:
    CheerSelectDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Returns -1 if not found.
    int8_t detect_slot(const ImageViewRGB32& screen) const;
    bool move_to_slot(VideoStream& stream, SwitchControllerContext& context, uint8_t slot) const;

private:
    WhiteButtonDetector m_info_button;
    GradientArrowDetector m_arrow;
};
class CheerSelectWatcher : public DetectorToFinder<CheerSelectDetector>{
public:
    CheerSelectWatcher(Color color)
        : DetectorToFinder("CheerSelectWatcher", std::chrono::milliseconds(250), color)
    {}
};



class TeraTargetSelectDetector : public StaticScreenDetector{
public:
    TeraTargetSelectDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Returns -1 if not found.
    //  Returns 0 if opponent.
    //  Returns 1 if left-most player.
    //  Returns 4 if right-most player.
    int8_t detect_slot(const ImageViewRGB32& screen) const;
    bool move_to_slot(VideoStream& stream, SwitchControllerContext& context, uint8_t slot) const;

private:
    GradientArrowDetector m_opponent;
    GradientArrowDetector m_player0;
    GradientArrowDetector m_player1;
    GradientArrowDetector m_player2;
    GradientArrowDetector m_player3;
};
class TargetSelectWatcher : public DetectorToFinder<TeraTargetSelectDetector>{
public:
    TargetSelectWatcher(Color color)
        : DetectorToFinder("TargetSelectWatcher", std::chrono::milliseconds(250), color)
    {}
};



class TeraCatchDetector : public StaticScreenDetector{
public:
    TeraCatchDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    bool move_to_slot(VideoStream& stream, SwitchControllerContext& context, uint8_t slot) const;

private:
    bool detect_slot(const ImageViewRGB32& screen, size_t index) const;

private:
    Color m_color;
    WhiteButtonDetector m_callouts_button;

    ImageFloatBox m_button[2];
    ImageFloatBox m_box_right[2];
    std::vector<GradientArrowDetector> m_arrow;
};
class TeraCatchWatcher : public DetectorToFinder<TeraCatchDetector>{
public:
    TeraCatchWatcher(Color color)
        : DetectorToFinder("TeraCatchWatcher", std::chrono::milliseconds(1000), color)
    {}
};





}
}
}
#endif
