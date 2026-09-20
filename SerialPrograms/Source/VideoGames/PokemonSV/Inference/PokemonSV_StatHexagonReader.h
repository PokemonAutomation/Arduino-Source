/*  Stat Hexagon Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_StatHexagonReader_H
#define PokemonAutomation_PokemonSV_StatHexagonReader_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "Pokemon/Pokemon_NatureChecker.h"

namespace PokemonAutomation{

class Logger;
struct ImageStats;

namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



class StatHexagonReader{
public:
    StatHexagonReader(
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
    );

    void make_overlays(VideoOverlaySet& items) const;

    NatureAdjustments read_nature(Logger& logger, const ImageViewRGB32& screen) const;
    int8_t read_level(Logger& logger, const ImageViewRGB32& screen) const;
    StatReads read_stats(Logger& logger, const ImageViewRGB32& screen) const;
    IvRanges calc_ivs(
        Logger& logger, const ImageViewRGB32& screen,
        const BaseStats& base_stats, const EVs& evs = EVs()
    ) const;


private:
    NatureAdjustment read_stat_adjustment(ImageStats& stats, const ImageFloatBox& box, const ImageViewRGB32& screen) const;
    int16_t read_stat(Logger& logger, const ImageFloatBox& box, const ImageViewRGB32& screen) const;


private:
    Color m_color;

    ImageFloatBox m_level;

//    ImageFloatBox m_ball_hp;
    ImageFloatBox m_ball_attack;
    ImageFloatBox m_ball_defense;
    ImageFloatBox m_ball_spatk;
    ImageFloatBox m_ball_spdef;
    ImageFloatBox m_ball_speed;

    ImageFloatBox m_stat_hp;
    ImageFloatBox m_stat_attack;
    ImageFloatBox m_stat_defense;
    ImageFloatBox m_stat_spatk;
    ImageFloatBox m_stat_spdef;
    ImageFloatBox m_stat_speed;
};


#if 0   //  This doesn't work due to HP and level reads being variable position.
class BoxStatsReader : public StatHexagonReader{
public:
    BoxStatsReader(Color color = COLOR_RED);
};
#endif

class SummaryStatsReader : public StatHexagonReader{
public:
    SummaryStatsReader(Color color = COLOR_RED);
};



}
}
}
#endif
