/*  Notification Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA_NotificationReader.h"

#include <iostream>
using std::cout;
using std::endl;

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
    LoggerQt& logger,
    Language language,
    const ConstImageRef& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        min_text_ratio, max_text_ratio
    );
}






NotificationReader::NotificationReader(LoggerQt& logger, Language language)
    : m_logger(logger)
    , m_language(language)
    , m_ocr_box(0.30, 0.138, 0.40, 0.036)
{}

void NotificationReader::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_ocr_box);
}
Notification NotificationReader::detect(const QImage& screen) const{
    ConstImageRef image = extract_box_reference(screen, m_ocr_box);


    //  Check if there anything that looks like text.
    size_t objects = 0;
    {
        PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(image, 0xff808080, 0xffffffff);
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
        {
            {0xff808080, 0xffffffff},
            {0xffa0a0a0, 0xffffffff},
        }
    );

    if (results.results.empty()){
        return Notification::NOTHING;
    }


#if 0
    const double max_log10p = -8.0;


//    image = image.convertToFormat(QImage::Format::Format_ARGB32);
    QImage image_ocr((int)image.width(), (int)image.height(), QImage::Format_ARGB32);

    OCR::StringMatchResult results;
    bool good = false;

    std::vector<uint32_t> filters{
        0xff808080,
        0xffa0a0a0,
    };
    for (uint32_t filter : filters){
        Kernels::filter_rgb32_range(
            image.data(), image.bytes_per_row(), image.width(), image.height(),
            (uint32_t*)image_ocr.bits(), image_ocr.bytesPerLine(), filter, 0xffffffff, 0xff000000, false
        );

        QString text = OCR::ocr_read(m_language, image_ocr);
        results = NotificationOCR::instance().match_substring(m_language, text, 4.0);

        results.clear_beyond_log10p(-2.0);
        results.log(m_logger, max_log10p);

        //  No reads at all.
        if (results.results.empty()){
            continue;
        }

        //  Ambiguous.
        if (results.results.size() > 1){
            m_logger.log("Ambiguous read result.", COLOR_RED);
            continue;
        }

        //  Result found!
        if (results.results.begin()->first < max_log10p){
            good = true;
            break;
        }
    }

    if (!good){
        return Notification::NOTHING;
    }
#endif

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




NotificationDetector::NotificationDetector(LoggerQt& logger, Language language)
    : VisualInferenceCallback("NotificationDetector")
    , m_reader(logger, language)
    , m_last(Notification::NOTHING)
    , m_last_check(WallClock::min())
{}

void NotificationDetector::make_overlays(VideoOverlaySet& items) const{
    m_reader.make_overlays(items);
}

bool NotificationDetector::process_frame(const QImage& frame, WallClock timestamp){
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
