/*  Outbreak Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ImageTools/ImageStats.h"
//#include "CommonFramework/OCR/OCR_Filtering.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_OutbreakReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


OutbreakReader::OutbreakReader(LoggerQt& logger, Language language, VideoOverlay& overlay)
    : m_logger(logger)
    , m_language(language)
    , m_dialog_box0(overlay, 0.030, 0.177, 0.020, 0.038)
    , m_dialog_box1(overlay, 0.030, 0.225, 0.020, 0.038)
    , m_text_box(overlay, 0.050, 0.177, 0.200, 0.038)
{}

OCR::StringMatchResult OutbreakReader::read(const QImage& screen) const{
    OCR::StringMatchResult result;
    ImageStats box0 = image_stats(extract_box_reference(screen, m_dialog_box0));
    ImageStats box1 = image_stats(extract_box_reference(screen, m_dialog_box1));
    double distance = euclidean_distance(box0.average, box1.average);
    if (distance < 20){
        m_logger.log("No outbreak found.", COLOR_ORANGE);
        return result;
    }

    ConstImageRef image = extract_box_reference(screen, m_text_box);
    QImage filtered = image.to_qimage();

#if 1
    //  TODO: Clean this shit up.
    size_t count;
    Kernels::filter_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        count, (uint32_t*)filtered.bits(), filtered.bytesPerLine(), 0xff808080, 0xffffffff, 0xffffffff, false
    );
    Kernels::filter_rgb32_range(
        (const uint32_t*)filtered.constBits(), filtered.bytesPerLine(), filtered.width(), filtered.height(),
        count, (uint32_t*)filtered.bits(), filtered.bytesPerLine(), 0xffffffff, 0xffffffff, 0xff000000, false
    );
#else
    OCR::filter_smart(filtered);
#endif

//    filtered.save("test1.png");

    result = Pokemon::PokemonNameReader::instance().read_substring(m_logger, m_language, filtered);
    result.clear_beyond_log10p(Pokemon::PokemonNameReader::MAX_LOG10P);
    if (result.results.empty()){
        dump_image(m_logger, ProgramInfo(), "OutbreakReader", screen);
    }

    return result;
}


}
}
}
