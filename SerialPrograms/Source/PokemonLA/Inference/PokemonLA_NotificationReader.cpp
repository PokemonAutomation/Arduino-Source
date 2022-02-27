/*  Notification Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
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



NotificationReader::NotificationReader(LoggerQt& logger, Language language)
    : m_logger(logger)
    , m_language(language)
    , m_ocr_box(0.30, 0.138, 0.40, 0.036)
{}

void NotificationReader::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_ocr_box);
}
Notification NotificationReader::detect(const QImage& screen) const{
    const double max_log10p = -8.0;

    QImage image = extract_box(screen, m_ocr_box);


    //  Check if there anything that looks like text.
    size_t objects = 0;
    {
        PackedBinaryMatrix matrix(image.width(), image.height());
        compress_rgb32_to_binary_range(
            matrix,
            (uint32_t*)image.bits(), image.bytesPerLine(),
            0xff808080, 0xffffffff
        );
        WaterFillIterator iter(matrix, 20);
        WaterfillObject object;
        while (iter.find_next(object)){
            objects++;
        }
        if (objects < 20){
            return Notification::NOTHING;
        }
    }


    image = image.convertToFormat(QImage::Format::Format_ARGB32);
    QImage image_ocr = image;


    m_logger.log("NotificationReader: Possible text found (" + std::to_string(objects) + " objects). Attempting to read it...", COLOR_PURPLE);

    OCR::StringMatchResult results;
    bool good = false;

    std::vector<uint32_t> filters{
        0xff808080,
        0xffa0a0a0,
    };
    for (uint32_t filter : filters){
        Kernels::filter_rgb32_range(
            (const uint32_t*)image.constBits(), image.bytesPerLine(), image.width(), image.height(),
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
    , m_last_check(std::chrono::system_clock::time_point::min())
{}

void NotificationDetector::make_overlays(VideoOverlaySet& items) const{
    m_reader.make_overlays(items);
}

bool NotificationDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    //  Throttle this to 1/sec.
    auto now = std::chrono::system_clock::now();
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
