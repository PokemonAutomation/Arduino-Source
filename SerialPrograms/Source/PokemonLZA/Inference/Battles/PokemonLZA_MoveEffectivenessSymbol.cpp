/*  Move Effectiveness Symbol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
//#include "CommonFramework/Logging/Logger.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonLZA_MoveEffectivenessSymbol.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


const char* template_path(MoveEffectivenessSymbol symbol){
    switch (symbol){
    case MoveEffectivenessSymbol::NoEffect:
        return "PokemonLZA/MoveEffectiveness/MoveEffectiveness-NoEffect.png";
    case MoveEffectivenessSymbol::NotVeryEffective:
        return "PokemonLZA/MoveEffectiveness/MoveEffectiveness-NotVeryEffective.png";
    case MoveEffectivenessSymbol::Normal:
        return "PokemonLZA/MoveEffectiveness/MoveEffectiveness-Normal.png";
    case MoveEffectivenessSymbol::SuperEffective:
        return "PokemonLZA/MoveEffectiveness/MoveEffectiveness-SuperEffective.png";
    default:
        return "";
    }
}


MoveEffectivenessSymbolMatcher::MoveEffectivenessSymbolMatcher(MoveEffectivenessSymbol symbol)
    : WaterfillTemplateMatcher(
        template_path(symbol),
        Color(0xffe0e0e0),
        Color(0xffffffff),
        100
    )
{}



MoveEffectivenessSymbolDetector::MoveEffectivenessSymbolDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_box_top(0.780000, 0.750000, 0.025000, 0.041000)
    , m_box_left(0.750000, 0.800000, 0.025000, 0.041000)
    , m_box_right(0.806000, 0.800000, 0.025000, 0.041000)
    , m_box_bottom(0.780000, 0.850000, 0.025000, 0.041000)
    , m_overlay(overlay)
{}

void MoveEffectivenessSymbolDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, ImageFloatBox(0.729553, 0.704854, 0.125409, 0.223301));
//    items.add(m_color, m_box_top);
//    items.add(m_color, m_box_left);
//    items.add(m_color, m_box_right);
//    items.add(m_color, m_box_bottom);
}
bool MoveEffectivenessSymbolDetector::detect(const ImageViewRGB32& screen){
    bool found = false;
    found |= detect_box(0, screen, m_box_top);
    found |= detect_box(1, screen, m_box_left);
    found |= detect_box(2, screen, m_box_right);
    found |= detect_box(3, screen, m_box_bottom);
    return found;
}
bool MoveEffectivenessSymbolDetector::detect_box(
    int index,
    const ImageViewRGB32& screen,
    const ImageFloatBox& box
){
    using namespace Kernels::Waterfill;

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS{
//        {0xff808080, 0xffffffff},
//        {0xff909090, 0xffffffff},
//        {0xffa0a0a0, 0xffffffff},
//        {0xffb0b0b0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
        {0xffd0d0d0, 0xffffffff},
        {0xffe0e0e0, 0xffffffff},
        {0xfff0f0f0, 0xffffffff},
    };

    ImageViewRGB32 image = extract_box_reference(screen, box);

    double best_rmsd = 99999;
    MoveEffectivenessSymbol best_symbol = MoveEffectivenessSymbol::None;
    ImagePixelBox best_box;
    Color best_color;
    const char* best_label = nullptr;

    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(image, FILTERS);
    auto session = make_WaterfillSession();
    for (PackedBinaryMatrix& matrix : matrices){
        ImageRGB32 filtered = image.copy();
        filter_by_mask(matrix, filtered, Color(0xff000000), true);

        session->set_source(matrix);
        auto iter = session->make_iterator(50);

        WaterfillObject object;
        while (iter->find_next(object, false)){
            ImageViewRGB32 cropped = extract_box_reference(filtered, object);
            {
                double rmsd = MoveEffectivenessSymbolMatcher::NoEffect().rmsd(screen.size(), cropped);
                if (best_rmsd > rmsd){
                    best_rmsd = rmsd;
                    best_symbol = MoveEffectivenessSymbol::NoEffect;
                    best_box = object;
                    best_color = COLOR_RED;
                    best_label = "No Effect";
                }
            }
            {
                double rmsd = MoveEffectivenessSymbolMatcher::NotVeryEffective().rmsd(screen.size(), cropped);
                if (best_rmsd > rmsd){
                    best_rmsd = rmsd;
                    best_symbol = MoveEffectivenessSymbol::NotVeryEffective;
                    best_box = object;
                    best_color = COLOR_YELLOW;
                    best_label = "Not Very Effective";
                }
            }
            {
                double rmsd = MoveEffectivenessSymbolMatcher::Normal().rmsd(screen.size(), cropped);
                if (best_rmsd > rmsd){
                    best_rmsd = rmsd;
                    best_symbol = MoveEffectivenessSymbol::Normal;
                    best_box = object;
                    best_color = COLOR_GREEN;
                    best_label = "Effective";
                }
            }
            {
                double rmsd = MoveEffectivenessSymbolMatcher::SuperEffective().rmsd(screen.size(), cropped);
                if (best_rmsd > rmsd){
                    best_rmsd = rmsd;
                    best_symbol = MoveEffectivenessSymbol::SuperEffective;
                    best_box = object;
                    best_color = COLOR_BLUE;
                    best_label = "Super Effective";
                }
            }
        }
    }

//    cout << "best_symbol = " << (int)best_symbol << ", rmsd = " << best_rmsd << endl;

    if (best_rmsd > 150){
        m_last_detection[index] = MoveEffectivenessSymbol::None;
        m_last_detection_boxes[index].reset();
        return false;
    }else{
        m_last_detection[index] = best_symbol;
        m_last_detection_boxes[index].emplace(
            *m_overlay,
            best_color,
            translate_to_parent(screen, box, best_box),
            best_label
        );
//        global_logger_tagged().log("Detected!");
        return true;
    }
}



}
}
}
