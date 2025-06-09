/*  Normal Battle Menus
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_NormalBattleMenus_H
#define PokemonAutomation_PokemonSV_NormalBattleMenus_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogArrowDetector.h"


namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class NormalBattleMenuDetector : public StaticScreenDetector{
public:
    NormalBattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    //  Returns -1 if not found.
    int8_t detect_slot(const ImageViewRGB32& screen);
    bool move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot);

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

std::set<std::string> read_singles_opponent(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    Language language
);



class MoveSelectDetector : public StaticScreenDetector{
public:
    MoveSelectDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    //  Returns -1 if not found.
    int8_t detect_slot(const ImageViewRGB32& screen);
    bool move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot);

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
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};



class SwapMenuDetector : public StaticScreenDetector{
public:
    SwapMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    //  Returns -1 if not found.
    int8_t detect_slot(const ImageViewRGB32& screen) const;
    bool move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot) const;

private:
    GradientArrowDetector m_arrow;
};
class SwapMenuWatcher : public DetectorToFinder<SwapMenuDetector>{
public:
    SwapMenuWatcher(Color color)
        : DetectorToFinder("SwapMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};

class WipeoutDetector : public StaticScreenDetector{
public:
    WipeoutDetector(Color color = COLOR_CYAN);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    // return true if detects a black screen, black dialog box, and dialog arrow.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    BlackScreenDetector m_blackscreen;
    DialogBoxDetector m_dialog;
    DialogArrowDetector m_arrow_detector;
};
class WipeoutWatcher : public DetectorToFinder<WipeoutDetector>{
public:
    WipeoutWatcher(Color color = COLOR_CYAN)
        : DetectorToFinder("WipeoutWatcher", std::chrono::milliseconds(250), color)
    {}
};





}
}
}
#endif
