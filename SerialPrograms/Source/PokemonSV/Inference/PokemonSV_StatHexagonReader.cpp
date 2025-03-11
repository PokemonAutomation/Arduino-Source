/*  Stat Hexagon Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonSV_StatHexagonReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


StatHexagonReader::StatHexagonReader(
    Color color,
    const ImageFloatBox& level,
    const ImageFloatBox& ball_attack,
    const ImageFloatBox& ball_defense,
    const ImageFloatBox& ball_spatk,
    const ImageFloatBox& ball_spdef,
    const ImageFloatBox& ball_speed,
    const ImageFloatBox& stat_hp,
    const ImageFloatBox& stat_attack,
    const ImageFloatBox& stat_defense,
    const ImageFloatBox& stat_spatk,
    const ImageFloatBox& stat_spdef,
    const ImageFloatBox& stat_speed
)
    : m_color(color)
    , m_level(level)
    , m_ball_attack(ball_attack)
    , m_ball_defense(ball_defense)
    , m_ball_spatk(ball_spatk)
    , m_ball_spdef(ball_spdef)
    , m_ball_speed(ball_speed)
    , m_stat_hp(stat_hp)
    , m_stat_attack(stat_attack)
    , m_stat_defense(stat_defense)
    , m_stat_spatk(stat_spatk)
    , m_stat_spdef(stat_spdef)
    , m_stat_speed(stat_speed)
{}
void StatHexagonReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_level);
    items.add(m_color, m_ball_attack);
    items.add(m_color, m_ball_defense);
    items.add(m_color, m_ball_spatk);
    items.add(m_color, m_ball_spdef);
    items.add(m_color, m_ball_speed);
    items.add(m_color, m_stat_hp);
    items.add(m_color, m_stat_attack);
    items.add(m_color, m_stat_defense);
    items.add(m_color, m_stat_spatk);
    items.add(m_color, m_stat_spdef);
    items.add(m_color, m_stat_speed);
}
NatureAdjustment StatHexagonReader::read_stat_adjustment(ImageStats& stats, const ImageFloatBox& box, const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 region = extract_box_reference(screen, box);

//    cout << "--------------" << endl;
    {
        auto matrix = compress_rgb32_to_binary_range(region, 0xffc0c0c0, 0xffffffff);
        auto session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(20);

        WaterfillObject object;
        while (iter->find_next(object, false)){
//            cout << object.area << " : " << object.width() << " x " << object.height()
//                 << ", aspect = " << object.aspect_ratio() << ", area ratio = " << object.area_ratio() << endl;

            double aspect_ratio = object.aspect_ratio();
            if (!(0.8 < aspect_ratio && aspect_ratio < 1.2)){
                continue;
            }
            if (object.area_ratio() < 0.65){
                continue;
            }

            return NatureAdjustment::NEUTRAL;
        }
    }

    stats = image_stats(region);
//    cout << stats.average << "," << stats.stddev << endl;

    return stats.average.r > stats.average.b
        ? NatureAdjustment::POSITIVE
        : NatureAdjustment::NEGATIVE;
}
NatureAdjustments StatHexagonReader::read_nature(Logger& logger, const ImageViewRGB32& screen) const{
    NatureAdjustments ret;

    std::vector<std::pair<NatureAdjustment*, ImageStats>> non_neutral;

    ImageStats stats;

    ret.attack  = read_stat_adjustment(stats, m_ball_attack, screen);
    if (ret.attack != NatureAdjustment::NEUTRAL){
        non_neutral.emplace_back(&ret.attack, stats);
    }

    ret.defense = read_stat_adjustment(stats, m_ball_defense, screen);
    if (ret.defense != NatureAdjustment::NEUTRAL){
        non_neutral.emplace_back(&ret.defense, stats);
    }

    ret.spatk   = read_stat_adjustment(stats, m_ball_spatk, screen);
    if (ret.spatk != NatureAdjustment::NEUTRAL){
        non_neutral.emplace_back(&ret.spatk, stats);
    }

    ret.spdef   = read_stat_adjustment(stats, m_ball_spdef, screen);
    if (ret.spdef != NatureAdjustment::NEUTRAL){
        non_neutral.emplace_back(&ret.spdef, stats);
    }

    ret.speed   = read_stat_adjustment(stats, m_ball_speed, screen);
    if (ret.speed != NatureAdjustment::NEUTRAL){
        non_neutral.emplace_back(&ret.speed, stats);
    }

    if (non_neutral.size() == 0){
        return ret;
    }
    if (non_neutral.size() != 2){
        throw_and_log<OperationFailedException>(
            logger, ErrorReport::SEND_ERROR_REPORT,
            "Unable to read nature."
        );
    }

    if (*non_neutral[0].first != *non_neutral[1].first){
        return ret;
    }

//    cout << non_neutral[0].second.average << non_neutral[1].second.average << endl;

    if (non_neutral[0].second.average.r > non_neutral[1].second.average.r &&
        non_neutral[0].second.average.b < non_neutral[1].second.average.b
    ){
        *non_neutral[0].first = NatureAdjustment::POSITIVE;
        *non_neutral[1].first = NatureAdjustment::NEGATIVE;
        return ret;
    }
    if (non_neutral[0].second.average.r < non_neutral[1].second.average.r &&
        non_neutral[0].second.average.b > non_neutral[1].second.average.b
    ){
        *non_neutral[0].first = NatureAdjustment::NEGATIVE;
        *non_neutral[1].first = NatureAdjustment::POSITIVE;
        return ret;
    }

    throw_and_log<OperationFailedException>(
        logger, ErrorReport::SEND_ERROR_REPORT,
        "Unable to read nature."
    );
}


int8_t StatHexagonReader::read_level(Logger& logger, const ImageViewRGB32& screen) const{
    ImageViewRGB32 region = extract_box_reference(screen, m_level);
#if 0
    ImageRGB32 filtered = to_blackwhite_rgb32_range(region, 0xff808080, 0xffffffff, true);
    filtered = pad_image(filtered, filtered.height() / 2, 0xffffffff);

    int number = OCR::read_number(logger, filtered);
#else
    int number = OCR::read_number_waterfill(logger, region, 0xff808080, 0xffffffff);
#endif
    if (number < 0 || number > 100){
        number = -1;
    }
    return (int8_t)number;
}
int16_t StatHexagonReader::read_stat(Logger& logger, const ImageFloatBox& box, const ImageViewRGB32& screen) const{
    ImageViewRGB32 region = extract_box_reference(screen, box);
#if 0
    ImageRGB32 filtered = to_blackwhite_rgb32_range(region, 0xff808080, 0xffffffff, true);
    filtered = pad_image(filtered, filtered.height() / 2, 0xffffffff);

//    static int c = 0;
//    filtered.save("test-" + std::to_string(c++) + ".png");

    int number = OCR::read_number_waterfill(logger, filtered);
#else
    int number = OCR::read_number_waterfill(logger, region, 0xff808080, 0xffffffff);
#endif
    if (number < 5 || number > 999){
        number = -1;
    }
    return (int16_t)number;
}
StatReads StatHexagonReader::read_stats(Logger& logger, const ImageViewRGB32& screen) const{
    StatReads ret;

    //  TODO: Handle the slash. (99 / 100)
    ret.hp      = read_stat(logger, m_stat_hp, screen);

    ret.attack  = read_stat(logger, m_stat_attack, screen);
    ret.defense = read_stat(logger, m_stat_defense, screen);
    ret.spatk   = read_stat(logger, m_stat_spatk, screen);
    ret.spdef   = read_stat(logger, m_stat_spdef, screen);
    ret.speed   = read_stat(logger, m_stat_speed, screen);
    return ret;
}
IvRanges StatHexagonReader::calc_ivs(
    Logger& logger, const ImageViewRGB32& screen,
    const BaseStats& base_stats, const EVs& evs
) const{
    int16_t level = read_level(logger, screen);
    NatureAdjustments nature = read_nature(logger, screen);
    StatReads stats = read_stats(logger, screen);
    return calc_iv_ranges(base_stats, (uint8_t)level, evs, stats, nature);
}



#if 0
BoxStatsReader::BoxStatsReader(Color color)
    : StatHexagonReader(
        color,
//        {0.823, 0.244, 0.012, 0.022},
        {0.875, 0.294, 0.012, 0.022},
        {0.875, 0.402, 0.012, 0.022},
        {0.771, 0.294, 0.012, 0.022},
        {0.771, 0.402, 0.012, 0.022},
        {0.823, 0.453, 0.012, 0.022},
        {0.828, 0.198, 0.080, 0.045},
        {0.890, 0.408, 0.050, 0.045},
        {0.890, 0.198, 0.050, 0.045},
        {0.720, 0.408, 0.050, 0.045},
        {0.720, 0.198, 0.050, 0.045},
        {0.828, 0.475, 0.050, 0.045}
    )
{}
#endif

SummaryStatsReader::SummaryStatsReader(Color color)
    : StatHexagonReader(
        color,
        {0.125, 0.860, 0.050, 0.044},
//        {0.755, 0.253, 0.012, 0.022},
        {0.834, 0.335, 0.012, 0.022},
        {0.834, 0.483, 0.012, 0.022},
        {0.677, 0.335, 0.012, 0.022},
        {0.677, 0.483, 0.012, 0.022},
        {0.755, 0.565, 0.012, 0.022},
        {0.767, 0.205, 0.040, 0.050},
        {0.848, 0.340, 0.040, 0.050},
        {0.848, 0.520, 0.040, 0.050},
        {0.637, 0.340, 0.040, 0.050},
        {0.637, 0.520, 0.040, 0.050},
        {0.742, 0.620, 0.040, 0.050}
    )
{}






}
}
}
