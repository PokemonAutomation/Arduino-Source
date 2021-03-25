/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyEncounterDetector_H
#define PokemonAutomation_PokemonSwSh_ShinyEncounterDetector_H

#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyEncounterDetector{
public:
    static const InferenceBox REGULAR_BATTLE;
    static const InferenceBox RAID_BATTLE;

    enum Detection{
        NO_BATTLE_MENU,
        NOT_SHINY,
        STAR_SHINY,
        SQUARE_SHINY,
    };

public:
    ShinyEncounterDetector(
        VideoFeed& feed, Logger& logger,
        const InferenceBox& box,
        std::chrono::seconds timeout,
        double detection_threshold = 4.0
    );

    Detection detect(ProgramEnvironment& env);

private:
    VideoFeed& m_feed;
    Logger& m_logger;
    InferenceBoxScope m_box;
    std::chrono::seconds m_timeout;
    double m_detection_threshold;
    StandardBattleMenuDetector m_menu;
};




}
}
}
#endif
