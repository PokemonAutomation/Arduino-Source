/*  Normal Battle Menus
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_NormalBattleMenus_H
#define PokemonAutomation_PokemonSV_NormalBattleMenus_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class NormalBattleMenuDetector : public StaticScreenDetector{
public:
    NormalBattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Returns -1 if not found.
    int8_t detect_slot(const ImageViewRGB32& screen) const;
    bool move_to_slot(ConsoleHandle& console, BotBaseContext& context, uint8_t slot) const;

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
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    Language language
);



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







}
}
}
#endif
