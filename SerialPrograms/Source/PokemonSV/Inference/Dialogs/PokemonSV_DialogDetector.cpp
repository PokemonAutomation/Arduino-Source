/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_DialogArrowDetector.h"
#include "PokemonSV_GradientArrowDetector.h"
#include "PokemonSV_DialogDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


DialogBoxDetector::DialogBoxDetector(Color color, bool true_if_detected, DialogType type)
    : m_color(color)
    , m_true_if_detected(true_if_detected)
    , m_box_top(0.50, 0.74, 0.20, 0.01)
    , m_box_bot(0.30, 0.88, 0.40, 0.01)
    , m_border_top(0.24, 0.71, 0.52, 0.04)
    , m_border_bot(0.24, 0.88, 0.52, 0.04)
    , m_dialog_type(type)
{}
void DialogBoxDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_top);
    items.add(m_color, m_box_bot);
    items.add(m_color, m_border_top);
    items.add(m_color, m_border_bot);
}
bool DialogBoxDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats_box_top = image_stats(extract_box_reference(screen, m_box_top));
//    cout << stats_box_top.average << stats_box_top.stddev << endl;
    bool white;
    if (is_white(stats_box_top)){
        white = true;
        if (m_dialog_type == DialogType::DIALOG_BLACK){
            return !m_true_if_detected;
        }
    }else if (is_black(stats_box_top, 150)){
        white = false;
        if (m_dialog_type == DialogType::DIALOG_WHITE){
            return !m_true_if_detected;
        }
    }else{
        return !m_true_if_detected;
    }

//    cout << "white = " << white << endl;

    ImageStats stats_box_bot = image_stats(extract_box_reference(screen, m_box_bot));
//    cout << stats_box_bot.average << stats_box_bot.stddev << endl;
    if (white){
        if (!is_white(stats_box_bot)){
            return !m_true_if_detected;
        }
    }else{
        if (!is_black(stats_box_bot, 150)){
            return !m_true_if_detected;
        }
    }

    ImageStats stats_border_top = image_stats(extract_box_reference(screen, m_border_top));
//    cout << stats_border_top.average << stats_border_top.stddev << endl;
    if (stats_border_top.stddev.sum() < 50){
        return !m_true_if_detected;
    }

    ImageStats stats_border_bot = image_stats(extract_box_reference(screen, m_border_bot));
//    cout << stats_border_bot.average << stats_border_bot.stddev << endl;
    if (stats_border_bot.stddev.sum() < 20){
        return !m_true_if_detected;
    }

    return m_true_if_detected;
}




AdvanceDialogDetector::AdvanceDialogDetector(Color color, DialogType type)
    : m_box(color, true, type)
    , m_arrow(0.710, 0.850, 0.030, 0.045)
{}
void AdvanceDialogDetector::make_overlays(VideoOverlaySet& items) const{
    m_box.make_overlays(items);
    items.add(m_box.color(), m_arrow);
}
bool AdvanceDialogDetector::detect(const ImageViewRGB32& screen){
    if (!m_box.detect(screen)){
        return false;
    }

    DialogArrowDetector arrow_detector(COLOR_RED, m_arrow);
    return arrow_detector.detect(screen);
}




PromptDialogDetector::PromptDialogDetector(Color color)
    : PromptDialogDetector(color, {0.50, 0.40, 0.40, 0.50})
{}
PromptDialogDetector::PromptDialogDetector(Color color, const ImageFloatBox& arrow_box)
    : m_box(color)
    , m_gradient(arrow_box)
{}
void PromptDialogDetector::make_overlays(VideoOverlaySet& items) const{
    m_box.make_overlays(items);
    items.add(m_box.color(), m_gradient);
}
bool PromptDialogDetector::detect(const ImageViewRGB32& screen){
    if (!m_box.detect(screen)){
        return false;
    }

    GradientArrowDetector gradiant_detector(COLOR_RED, GradientArrowType::RIGHT, m_gradient);
    return gradiant_detector.detect(screen);
}









}
}
}
