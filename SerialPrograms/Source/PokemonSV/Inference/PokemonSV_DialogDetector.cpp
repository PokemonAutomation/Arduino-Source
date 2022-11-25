/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSV_DialogArrowDetector.h"
#include "PokemonSV_GradientArrowDetector.h"
#include "PokemonSV_DialogDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


AdvanceDialogDetector::AdvanceDialogDetector(Color color)
    : m_color(color)
    , m_box_top(0.50, 0.74, 0.20, 0.01)
    , m_box_bot(0.30, 0.88, 0.40, 0.01)
    , m_arrow(0.710, 0.850, 0.030, 0.042)
{}
void AdvanceDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_top);
    items.add(m_color, m_box_bot);
    items.add(m_color, m_arrow);
}
bool AdvanceDialogDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats stats_top = image_stats(extract_box_reference(screen, m_box_top));
    bool white;
    if (is_white(stats_top)){
        white = true;
    }else if (is_black(stats_top)){
        white = false;
    }else{
        return false;
    }

//    cout << "white = " << white << endl;

    ImageStats stats_bot = image_stats(extract_box_reference(screen, m_box_bot));
    if (white){
        if (!is_white(stats_bot)){
            return false;
        }
    }else{
        if (!is_black(stats_bot)){
            return false;
        }
    }
//    cout << "asdf" << endl;

    DialogArrowDetector arrow_detector(COLOR_RED, m_arrow);
    return arrow_detector.detect(screen);
}




PromptDialogDetector::PromptDialogDetector(Color color)
    : PromptDialogDetector(color, {0.50, 0.40, 0.40, 0.50})
{}
PromptDialogDetector::PromptDialogDetector(Color color, const ImageFloatBox& arrow_box)
    : m_color(color)
    , m_box_top(0.50, 0.74, 0.20, 0.01)
    , m_box_bot(0.30, 0.88, 0.40, 0.01)
    , m_gradient(arrow_box)
{}
void PromptDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_top);
    items.add(m_color, m_box_bot);
    items.add(m_color, m_gradient);
}
bool PromptDialogDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats stats_top = image_stats(extract_box_reference(screen, m_box_top));
    bool white;
    if (is_white(stats_top)){
        white = true;
    }else if (is_black(stats_top)){
        white = false;
    }else{
        return false;
    }

    ImageStats stats_bot = image_stats(extract_box_reference(screen, m_box_bot));
    if (white){
        if (!is_white(stats_bot)){
            return false;
        }
    }else{
        if (!is_black(stats_bot)){
            return false;
        }
    }

    GradientArrowDetector gradiant_detector(COLOR_RED, GradientArrowType::RIGHT, m_gradient);
    return gradiant_detector.detect(screen);
}









}
}
}
