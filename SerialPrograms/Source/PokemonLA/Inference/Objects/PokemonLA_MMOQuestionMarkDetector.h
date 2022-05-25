/*  MMO Question Mark Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  When you leave the village to choose a destination with MMO happening,
 *  There are some texts shown on the upper left corner of the screen:
 *  "Massive Massive Outbreak"
 *  "??????"
 *  This detector detects this string of six question marks.
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

    std::array<bool, 5> detect_MMO_on_hisui_map(const QImage& frame);

private:
    LoggerQt& m_logger;

    std::array<ImageFloatBox, 5> m_boxes;
};


bool detect_MMO_question_mark(const PokemonAutomation::ConstImageRef &image);


}
}
}
#endif
