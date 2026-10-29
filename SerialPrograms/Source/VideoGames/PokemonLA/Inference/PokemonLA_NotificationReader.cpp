/*  Notification Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonLA_NotificationReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;


const NotificationOCR& NotificationOCR::instance(){
    static NotificationOCR reader;
    return reader;
}

NotificationOCR::NotificationOCR()
    : SmallDictionaryMatcher("PokemonLA/NotificationOCR.json")
{}


OCR::StringMatchResult NotificationOCR::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        min_text_ratio, max_text_ratio
    );
}






NotificationReader::NotificationReader(Logger& logger, Language language)
    : m_logger(logger)
    , m_language(language)
    , m_ocr_box(0.30, 0.138, 0.40, 0.036)
{}

void NotificationReader::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_ocr_box);
}
Notification NotificationReader::detect(const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_ocr_box);


    //  Check if there anything that looks like text.
    size_t objects = 0;
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff808080, 0xffffffff);
        auto session = make_WaterfillSession(matrix);
        auto finder = session->make_iterator(20);
        WaterfillObject object;
        while (finder->find_next(object, false)){
            objects++;
        }
        if (objects < 20){
            return Notification::NOTHING;
        }
    }



    m_logger.log("NotificationReader: Possible text found (" + std::to_string(objects) + " objects). Attempting to read it...", COLOR_PURPLE);

    OCR::StringMatchResult results = NotificationOCR::instance().read_substring(
        m_logger, m_language, image,
        OCR::WHITE_TEXT_FILTERS()
    );

    if (results.results.empty()){
        return Notification::NOTHING;
    }

//    cout << results.results.begin()->second.token << endl;

    static std::map<std::string, Notification> MAP{
        {"distortion_forming",      Notification::DISTORTION_FORMING},
        {"distortion_appeared",     Notification::DISTORTION_APPEARED},
        {"distortion_faded",        Notification::DISTORTION_FADED},
        {"cannot_go_farther",       Notification::CANNOT_GO_FURTHER},
    };
    auto iter = MAP.find(results.results.begin()->second.token);
    if (iter != MAP.end()){
        return iter->second;
    }
    return Notification::NOTHING;
}




NotificationDetector::NotificationDetector(Logger& logger, Language language)
    : VisualInferenceCallback("NotificationDetector")
    , m_reader(logger, language)
    , m_last(Notification::NOTHING)
    , m_last_check(WallClock::min())
{}

void NotificationDetector::make_overlays(VideoOverlaySet& items) const{
    m_reader.make_overlays(items);
}

bool NotificationDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  Throttle this to 1/sec.
    auto now = current_time();
    if (m_last_check + std::chrono::milliseconds(1000) > now){
        return false;
    }
    m_last_check = now;

    Notification result = m_reader.detect(frame);
    m_last.store(result, std::memory_order_release);
    return result != Notification::NOTHING;
}











}
}
}
