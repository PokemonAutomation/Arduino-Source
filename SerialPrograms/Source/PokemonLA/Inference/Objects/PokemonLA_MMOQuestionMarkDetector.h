/*  MMO Question Mark Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect MMO question mark symbol.
 */

#ifndef PokemonAutomation_PokemonLA_MMOQuestionMarkDetector_H
#define PokemonAutomation_PokemonLA_MMOQuestionMarkDetector_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"

#include <array>

class QImage;

namespace PokemonAutomation{

class VideoOverlaySet;


namespace NintendoSwitch{
namespace PokemonLA{


class MMOQuestionMarkMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MMOQuestionMarkMatcher();
    static const MMOQuestionMarkMatcher& instance();
};



class MMOQuestionMarkDetector {
public:
    MMOQuestionMarkDetector(LoggerQt& logger);

    void make_overlays(VideoOverlaySet& items) const;

    // Detect the MMO question marks on the Hisui map when you leave village.
    // Return an array of bool, each bool is whether MMO appears on one of the 
    // wild region. The order of the bool is the same order as the game progession:
    // Fieldlands, Mirelands, Coastlands, Highlands, Icelands.
    std::array<bool, 5> detect_MMO_on_hisui_map(const QImage& frame);

private:
    LoggerQt& m_logger;
};

// Detect the presense of MM question mark on an image
bool detect_MMO_question_mark(const PokemonAutomation::ConstImageRef &image);

void add_MMO_detection_to_overlay(const std::array<bool, 5>& detection_result, VideoOverlaySet& items);


}
}
}
#endif
