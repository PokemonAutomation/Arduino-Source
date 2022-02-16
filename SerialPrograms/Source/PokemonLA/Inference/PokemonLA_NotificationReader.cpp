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



NotificationReader::NotificationReader(Logger& logger, Language language)
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

#if 1
    Kernels::filter_rgb32_range(
        (uint32_t*)image.bits(), image.bytesPerLine(), image.width(), image.height(),
        (uint32_t*)image_ocr.bits(), image_ocr.bytesPerLine(), 0xff000000, 0xff808080, 0xffffffff, false
    );
#else
    QImage image_bkg = image_ocr;

    Kernels::filter2_rgb32_range(
        (uint32_t*)image.bits(), image.bytesPerLine(), image.width(), image.height(),
        (uint32_t*)image_ocr.bits(), image_ocr.bytesPerLine(), 0xff000000, 0xff808080, 0xffffffff, false,
        (uint32_t*)image_bkg.bits(), image_bkg.bytesPerLine(), 0x00000000, 0xff808080, 0xffffffff, true
    );

//    image_bkg.save("background.png");

    //  Check if the background is reasonably solid.
    {
        ImageStats stats = image_stats(image_bkg);
//        cout << stats.average << stats.stddev << endl;

        double stddev = stats.stddev.sum();

        m_logger.log("NotificationReader: Background stddev = " + std::to_string(stddev), COLOR_PURPLE);

        if (stddev > 250){
            return Notification::NOTHING;
        }
    }
#endif

    m_logger.log("NotificationReader: Possible text found (" + std::to_string(objects) + " objects). Attempting to read it...", COLOR_PURPLE);

    //  Now attempt to OCR the text.
    QString text = OCR::ocr_read(Language::English, image_ocr);

    OCR::StringMatchResult results;
    NotificationOCR::instance().match_substring(results, m_language, text);

    results.clear_beyond_log10p(-2.0);
    results.log(m_logger, max_log10p);

    if (results.results.empty()){
        return Notification::NOTHING;
    }
    if (results.results.begin()->first > max_log10p){
        return Notification::NOTHING;
    }

    cout << results.results.begin()->second.token << endl;

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
{}

void NotificationDetector::make_overlays(VideoOverlaySet& items) const{
    m_reader.make_overlays(items);
}

bool NotificationDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    Notification result = m_reader.detect(frame);
    m_last.store(result, std::memory_order_release);
    return result != Notification::NOTHING;
}











}
}
}
