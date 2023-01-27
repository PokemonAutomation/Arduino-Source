/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BattleMenuDetector_H
#define PokemonAutomation_PokemonSV_BattleMenuDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{



class NormalBattleMenuDetector : public StaticScreenDetector{
public:
    NormalBattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

//    //  Returns -1 if not found.
//    int8_t detect_slot(const ImageViewRGB32& screen) const;
//    bool move_to_slot(ConsoleHandle& console, BotBaseContext& context, uint8_t slot) const;

private:
    WhiteButtonDetector m_status_button;
    GradientArrowDetector m_arrow;
};
class NormalBattleMenuWatcher : public DetectorToFinder<NormalBattleMenuDetector>{
public:
    NormalBattleMenuWatcher(Color color)
        : DetectorToFinder("NormalBattleMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};



class TeraBattleMenuDetector : public StaticScreenDetector{
public:
    TeraBattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Returns -1 if not found.
    int8_t detect_slot(const ImageViewRGB32& screen) const;
    bool move_to_slot(ConsoleHandle& console, BotBaseContext& context, uint8_t slot) const;

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
    bool move_to_slot(ConsoleHandle& console, BotBaseContext& context, uint8_t slot) const;

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



class MoveSelectDetector : public StaticScreenDetector{
public:
    MoveSelectDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Returns -1 if not found.
    int8_t detect_slot(const ImageViewRGB32& screen) const;
    bool move_to_slot(ConsoleHandle& console, BotBaseContext& context, uint8_t slot) const;

private:
    WhiteButtonDetector m_info_button;
    GradientArrowDetector m_arrow;
};
class MoveSelectWatcher : public DetectorToFinder<MoveSelectDetector>{
public:
    MoveSelectWatcher(Color color)
        : DetectorToFinder("MoveSelectWatcher", std::chrono::milliseconds(250), color)
    {}
};



class TerastallizingDetector : public StaticScreenDetector{
public:
    TerastallizingDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};



class TargetSelectDetector : public StaticScreenDetector{
public:
    TargetSelectDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Returns -1 if not found.
    //  Returns 0 if opponent.
    //  Returns 1 if left-most player.
    //  Returns 4 if right-most player.
    int8_t detect_slot(const ImageViewRGB32& screen) const;
    bool move_to_slot(ConsoleHandle& console, BotBaseContext& context, uint8_t slot) const;

private:
    GradientArrowDetector m_opponent;
    GradientArrowDetector m_player0;
    GradientArrowDetector m_player1;
    GradientArrowDetector m_player2;
    GradientArrowDetector m_player3;
};
class TargetSelectWatcher : public DetectorToFinder<TargetSelectDetector>{
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

private:
    Color m_color;
    WhiteButtonDetector m_callouts_button;
    ImageFloatBox m_button;
    ImageFloatBox m_box_right;
    GradientArrowDetector m_arrow;
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
