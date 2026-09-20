/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MainMenuDetector_H
#define PokemonAutomation_PokemonSwSh_MainMenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class MainMenuDetector : public StaticScreenDetector{
public:
    MainMenuDetector(Color color = COLOR_BLUE);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_top_red;
    ImageFloatBox m_top_left;
    ImageFloatBox m_bottom_left;
};
class MainMenuWatcher : public DetectorToFinder<MainMenuDetector>{
public:
    MainMenuWatcher(Color color = COLOR_BLUE)
         : DetectorToFinder("MainMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};



class PartyMenuDetector : public StaticScreenDetector{
public:
    PartyMenuDetector(Color color = COLOR_GREEN);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_top_right;
    ImageFloatBox m_top_dark_red;
    ImageFloatBox m_bottom_dark_red;
    ImageFloatBox m_left;
};
class PartyMenuWatcher : public DetectorToFinder<PartyMenuDetector>{
public:
    PartyMenuWatcher(Color color = COLOR_GREEN)
         : DetectorToFinder("PartyMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};



class BoxMenuDetector : public StaticScreenDetector{
public:
    BoxMenuDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_left_green;
    ImageFloatBox m_top_white;
    ImageFloatBox m_bottom_white;
};
class BoxMenuWatcher : public DetectorToFinder<BoxMenuDetector>{
public:
    BoxMenuWatcher(Color color = COLOR_RED)
         : DetectorToFinder("BoxMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};









}
}
}
#endif
